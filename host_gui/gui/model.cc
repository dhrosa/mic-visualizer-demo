#include "model.h"

#include <absl/log/log.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <ranges>

#include "audio/source.h"
#include "audio/spectrum.h"
#include "image/lut.h"
#include "image/qimage_eigen.h"

Model::Model(double sample_rate, std::size_t fft_window_size)
    : sample_rate_(sample_rate),
      fft_window_size_(fft_window_size),
      frequency_bins_(FrequencyBins(fft_window_size, sample_rate)),
      data_(768, frequency_bins_.size()) {}

absl::Duration Model::TimeDelta(std::int64_t n) const {
  return absl::Seconds(n * fft_window_size_) / sample_rate_;
}

void Model::AppendSpectrum(Buffer<double> spectrum) {
  std::ranges::for_each(spectrum, [&](double& v) {
    v = std::log2(v + 1);
    min_value_ = std::min(v, min_value_);
    max_value_ = std::max(v, max_value_);
  });
  data_.AppendColumn(spectrum);
}

absl::AnyInvocable<void(QImage&) &&> Model::Renderer() {
  return [&](QImage& image) {
    auto lut = active_colormap_->entries;

    // We render the data upside-down so that higher frequencies are on the
    // top. Note: our image has the opposite orientation compared to Eigen
    // convention.
    auto dest = EigenView(image).colwise().reverse();

    auto newer = data_.Newer();
    auto older = data_.Older();

    LutMap(newer, dest.rightCols(newer.cols()), lut, min_value_, max_value_);
    LutMap(older, dest.leftCols(older.cols()), lut, min_value_, max_value_);
  };
}

AsyncGenerator<absl::AnyInvocable<void(QImage&) &&>> Model::Run() {
  auto source = RampSource({.sample_rate = sample_rate_,
                            .ramp_period = absl::Seconds(10),
                            .frequency_min = 100,
                            .frequency_max = 5000,
                            .pacing = SimulatedSourcePacing::kRealTime});
  auto spectra = PowerSpectrum(
      {.sample_rate = sample_rate_, .window_size = fft_window_size_},
      std::move(source));

  while (Buffer<double>* spectrum = co_await spectra) {
    AppendSpectrum(std::move(*spectrum));
    co_yield Renderer();
  }
}
