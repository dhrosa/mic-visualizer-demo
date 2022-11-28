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

  std::vector<std::complex<double>> spectrum;
  fftw_plan plan = fftw_plan_dft_1d(
      n, reinterpret_cast<fftw_complex*>(complex_samples.data()),
      reinterpret_cast<fftw_complex*>(spectrum.data()), FFTW_FORWARD,
      FFTW_ESTIMATE);
  auto plan_cleanup = [&]() { fftw_destroy_plan(plan); };

  fftw_execute(plan);
  return spectrum;
}
