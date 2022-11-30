#include "spectrum.h"

#include <absl/cleanup/cleanup.h>
#include <fftw3.h>

#include <algorithm>
#include <complex>
#include <memory>
#include <ranges>

namespace {

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

Buffer<std::complex<double>> Spectrum(std::span<const std::int16_t> samples,
                                      std::span<const double> window) {
  const std::size_t n = samples.size();
  // TODO(dhrosa): This can be an in-place FFT, as this buffer is only used
  // within this function.
  Buffer<std::complex<double>> complex_samples = FftwBuffer(n);
  std::ranges::transform(samples, window, complex_samples.begin(),
                         [](std::int16_t s, double w) { return w * s; });

  Buffer<std::complex<double>> spectrum = FftwBuffer(n);
  fftw_plan plan = fftw_plan_dft_1d(
      n, reinterpret_cast<fftw_complex*>(complex_samples.data()),
      reinterpret_cast<fftw_complex*>(spectrum.data()), FFTW_FORWARD,
      FFTW_ESTIMATE);
  auto plan_cleanup = absl::MakeCleanup([&]() { fftw_destroy_plan(plan); });

  fftw_execute(plan);
  return spectrum;
}

template <typename T>
Generator<std::vector<T>> ChunkedSamples(std::size_t n,
                                         Generator<Buffer<T>> source) {
  std::vector<T> chunk(n);
  std::span<T> unfilled_chunk_span(chunk);
  auto source_iter = source.begin();
  std::span<T> current_source_span = source_iter->span();
  while (true) {
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
    if (current_source_span.empty()) {
      ++source_iter;
      current_source_span = source_iter->span();
    }
  }
}

void CheckEven(std::size_t n) {
  if (n % 2 != 0) {
    throw std::invalid_argument("FFT length must be even. Got: " +
                                std::to_string(n));
  }
}
}  // namespace

// PSD scaling based off of https://dsp.stackexchange.com/a/32205

Buffer<double> PowerSpectrum(std::span<const std::int16_t> samples, double fs) {
  const std::size_t n = samples.size();
  CheckEven(n);
  const std::vector<double> window = Window(n);
  Buffer<std::complex<double>> spectrum = Spectrum(samples, window);
  const std::size_t norm = n * n;
  // DC bin and nyquist bins are the only bins that don't have a conjugate pair.
  const std::size_t conjugate_bin_count = (n - 2) / 2;
  const std::size_t nyquist_index = n / 2;

  const double psd_scale_factor = ScaleFactor(window) / (2 * fs);
  // TODO(dhrosa): We could reuse the buffer returned by Spectrum().
  auto power_spectrum = Buffer<double>::Uninitialized(2 + conjugate_bin_count);
  power_spectrum.front() = psd_scale_factor * std::norm(spectrum[0]);
  power_spectrum.back() = psd_scale_factor * std::norm(spectrum[nyquist_index]);
  auto positive_ac =
    std::move(spectrum) | std::views::drop(1) | std::views::take(conjugate_bin_count);


  
  std::ranges::transform(positive_ac, power_spectrum.begin() + 1,
                         [&](std::complex<double> s) {
                           return 2 * psd_scale_factor * std::norm(s);
                         });
  return power_spectrum;
}

std::vector<double> FrequencyBins(std::size_t n, double fs) {
  CheckEven(n);
  std::vector<double> bins(n / 2 + 1);
  for (std::size_t i = 0; i < bins.size(); ++i) {
    bins[i] = fs * i / n;
  }
  return bins;
}

Generator<Buffer<double>> PowerSpectrum(
    double sample_rate, std::size_t window_size,
    Generator<Buffer<std::int16_t>> source) {
  CheckEven(window_size);
  for (std::vector<std::int16_t> frame : ChunkedSamples(window_size, source)) {
    co_yield Buffer<double>();
  }
}
