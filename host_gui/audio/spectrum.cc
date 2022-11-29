#include "spectrum.h"

#include <absl/cleanup/cleanup.h>
#include <fftw3.h>

#include <algorithm>
#include <complex>
#include <ranges>

namespace {
std::vector<std::complex<double>> Spectrum(
    std::span<const std::int16_t> samples) {
  const std::size_t n = samples.size();
  std::vector<std::complex<double>> complex_samples(n);
  std::transform(samples.begin(), samples.end(), complex_samples.begin(),
                 [](auto x) { return static_cast<double>(x); });

  std::vector<std::complex<double>> spectrum(n);
  fftw_plan plan = fftw_plan_dft_1d(
      n, reinterpret_cast<fftw_complex*>(complex_samples.data()),
      reinterpret_cast<fftw_complex*>(spectrum.data()), FFTW_FORWARD,
      FFTW_ESTIMATE);
  auto plan_cleanup = absl::MakeCleanup([&]() { fftw_destroy_plan(plan); });

  fftw_execute(plan);
  return spectrum;
}

void CheckEven(std::size_t n) {
  if (n % 2 != 0) {
    throw std::invalid_argument("FFT length must be even. Got: " +
                                std::to_string(n));
  }
}
}  // namespace

std::vector<double> PowerSpectrum(std::span<const std::int16_t> samples) {
  const std::size_t n = samples.size();
  CheckEven(n);
  const std::vector<std::complex<double>> spectrum = Spectrum(samples);
  const std::size_t norm = n * n;
  // DC bin and nyquist bins are the only bins that don't have a conjugate pair
  const std::size_t conjugate_bin_count = (n - 2) / 2;
  const std::size_t nyquist_index = n / 2;

  auto positive_ac =
      spectrum | std::views::drop(1) | std::views::take(conjugate_bin_count);

  auto power = [&](auto x) { return 4 * std::norm(x) / norm; };

  std::vector<double> power_spectrum(2 + conjugate_bin_count);
  power_spectrum.front() = power(spectrum[0]);
  power_spectrum.back() = power(spectrum[nyquist_index]);

  std::ranges::transform(positive_ac, ++power_spectrum.begin(), power);

  return power_spectrum;
}

std::vector<double> FrequencyBins(std::size_t n, double fs) {
  CheckEven(n);
  std::vector<double> bins(n / 2 + 1);
  for (std::size_t i = 0; i < n / 2 + 1; ++i) {
    bins[i] = fs * i / n;
  }
  return bins;
}
