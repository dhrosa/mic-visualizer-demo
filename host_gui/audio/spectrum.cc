#include "spectrum.h"

#include <fftw3.h>

#include <algorithm>
#include <complex>
#include <memory>
#include <ranges>

namespace {

void CheckEven(std::size_t n) {
  if (n % 2 != 0) {
    throw std::invalid_argument("FFT length must be even. Got: " +
                                std::to_string(n));
  }
}

// Simple rectangular window.
//
// TODO(dhrosa): Replace with more general window functions.
std::vector<double> Window(std::size_t n) { return std::vector<double>(n, 1); }

double ScaleFactor(std::span<const double> window) {
  double factor = 0;
  for (double w : window) {
    factor += w * w;
  }
  return factor / (window.size() * window.size());
}

Buffer<std::complex<double>> FftwBuffer(std::size_t n) {
  auto* raw = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(n));
  return Buffer<std::complex<double>>({raw, n}, [raw] { fftw_free(raw); });
}

struct Plan {
  fftw_plan plan;
  ~Plan() { fftw_destroy_plan(plan); }
};

// Create a plan for an in-place complex-to-complex FFT.
Plan CreatePlan(std::size_t n) {
  auto fake_buffer = FftwBuffer(n);
  return {fftw_plan_dft_1d(n, reinterpret_cast<fftw_complex*>(fake_buffer.data()),
                           reinterpret_cast<fftw_complex*>(fake_buffer.data()),
                           FFTW_FORWARD, FFTW_ESTIMATE)};
}

Buffer<std::complex<double>> Spectrum(fftw_plan plan,
                                      std::span<const double> window,
                                      std::span<const std::int16_t> samples) {
  const std::size_t n = samples.size();
  Buffer<std::complex<double>> buffer = FftwBuffer(n);
  std::ranges::transform(samples, window, buffer.begin(),
                         [](std::int16_t s, double w) -> std::complex<double> { return w * s; });
  auto* buffer_data = reinterpret_cast<fftw_complex*>(buffer.data());
  // In-place FFT.
  fftw_execute_dft(plan, buffer_data, buffer_data);
  return buffer;
}

template <typename T>
AsyncGenerator<std::vector<T>> ChunkedSamples(
    std::size_t n, AsyncGenerator<Buffer<T>> source) {
  std::vector<T> chunk(n);
  std::span<T> unfilled_chunk_span(chunk);
  Buffer<T>* source_frame = co_await source;
  if (!source_frame) {
    co_return;
  }
  std::span<T> current_source_span = source_frame->span();
  while (true) {
    if (current_source_span.empty()) {
      source_frame = co_await source;
      if (!source_frame) {
        co_return;
      }
      current_source_span = source_frame->span();
    }
    const std::size_t copy_count =
        std::min(unfilled_chunk_span.size(), current_source_span.size());
    std::ranges::copy(current_source_span.first(copy_count),
                      unfilled_chunk_span.begin());
    current_source_span = current_source_span.subspan(copy_count);
    unfilled_chunk_span = unfilled_chunk_span.subspan(copy_count);
    if (unfilled_chunk_span.empty()) {
      co_yield chunk;
      unfilled_chunk_span = chunk;
    }
  }
}

// PSD scaling based off of https://dsp.stackexchange.com/a/32205 and
// https://dsp.stackexchange.com/a/47603
Buffer<double> SingleFramePowerSpectrum(fftw_plan plan,
                                        std::span<const double> window,
                                        double psd_scale_factor,
                                        std::span<const std::int16_t> samples) {
  const std::size_t n = samples.size();
  CheckEven(n);
  Buffer<std::complex<double>> spectrum = Spectrum(plan, window, samples);
  // DC bin and nyquist bins are the only bins that don't have a conjugate pair.
  const std::size_t conjugate_bin_count = (n - 2) / 2;
  const std::size_t nyquist_index = n / 2;

  // TODO(dhrosa): We could reuse the buffer returned by Spectrum().
  auto power_spectrum = Buffer<double>::Uninitialized(2 + conjugate_bin_count);
  power_spectrum.front() = psd_scale_factor * std::norm(spectrum[0]);
  power_spectrum.back() = psd_scale_factor * std::norm(spectrum[nyquist_index]);
  auto positive_ac = std::move(spectrum) | std::views::drop(1) |
                     std::views::take(conjugate_bin_count);

  std::ranges::transform(positive_ac, power_spectrum.begin() + 1,
                         [&](std::complex<double> s) {
                           return 2 * psd_scale_factor * std::norm(s);
                         });
  return power_spectrum;
}
}  // namespace

std::vector<double> FrequencyBins(std::size_t n, double fs) {
  CheckEven(n);
  std::vector<double> bins(n / 2 + 1);
  for (std::size_t i = 0; i < bins.size(); ++i) {
    bins[i] = fs * i / n;
  }
  return bins;
}

AsyncGenerator<Buffer<double>> PowerSpectrum(
    double sample_rate, std::size_t window_size,
    AsyncGenerator<Buffer<std::int16_t>> source) {
  CheckEven(window_size);
  const std::vector<double> window = Window(window_size);
  const double psd_scale_factor = ScaleFactor(window) / (2 * sample_rate);
  const Plan plan = CreatePlan(window_size);

  auto chunked_samples = ChunkedSamples(window_size, std::move(source));
  while (std::vector<std::int16_t>* frame = co_await chunked_samples) {
    co_yield SingleFramePowerSpectrum(plan.plan, window, psd_scale_factor,
                                      *frame);
  }
}
