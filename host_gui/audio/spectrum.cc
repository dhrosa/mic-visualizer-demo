#include "spectrum.h"

#include <absl/cleanup/cleanup.h>
#include <fftw3.h>

#include <algorithm>
#include <ranges>

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

std::vector<double> PowerSpectrum(std::span<const std::int16_t> samples) {
  const std::vector<std::complex<double>> spectrum = Spectrum(samples);
  const std::size_t n = samples.size();
  // DC bin and nyquist bins are the only bins that don't have a conjugate pair
  const std::size_t conjugate_bin_count = (n - 2) / 2;
  const std::size_t nyquist_index = 1 + conjugate_bin_count;

  auto positive_ac =
      spectrum | std::views::drop(1) | std::views::take(conjugate_bin_count);

  std::vector<double> power_spectrum(2 + conjugate_bin_count);
  power_spectrum[0] = spectrum[0].real();
  power_spectrum[nyquist_index] = spectrum[nyquist_index].real();

  std::ranges::transform(positive_ac, ++power_spectrum.begin(),
                         [](auto x) { return x.real() * x.real(); });

  return power_spectrum;
}
