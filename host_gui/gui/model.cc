#include "model.h"

#include <absl/log/log.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <ranges>

#include "audio/source.h"
#include "audio/spectrum.h"
#include "image/lut.h"
#include "image/qimage_eigen.h"

Model::Model() : Model(Options()) {}

Model::Model(const Options& options)
    : sample_rate_(options.sample_rate),
      fft_window_size_(options.fft_window_size),
      refresh_rate_(options.refresh_rate),
      frequency_bins_(::FrequencyBins(fft_window_size_, sample_rate_)),
      width_(1440),
      height_(frequency_bins_.size()),
      spectrum_data_(width_, height_),
      indexed_data_(width_, height_) {}

absl::Duration Model::TimeDelta(std::int64_t n) const {
  return absl::Seconds(n * fft_window_size_) / sample_rate_;
}

void Model::AppendSpectrum(Buffer<double> spectrum) {
  std::ranges::for_each(spectrum, [&](double& v) {
    v = std::log2(v + 1);
    min_value_ = std::min(v, min_value_);
    max_value_ = std::max(v, max_value_);
  });
  // TODO(dhrosa): Expose a CircularBuffer method to directly write to a new
  // column.
  auto indexed = Buffer<std::uint8_t>::Uninitialized(spectrum.size());
  ToIndexed(spectrum, indexed, min_value_, max_value_);

  spectrum_data_.AppendColumn(spectrum);
  indexed_data_.AppendColumn(indexed);
}

QImage Model::Render() {
  QImage image(imageSize(), QImage::Format_RGB32);
  auto lut = active_colormap_->entries;

  // We render the data upside-down so that higher frequencies are on the
  // top. Note: our image has the opposite orientation compared to Eigen
  // convention.
  auto dest = EigenView(image).colwise().reverse();

  auto newer = indexed_data_.Newer();
  auto older = indexed_data_.Older();

  LutMap(newer, dest.rightCols(newer.cols()), lut);
  LutMap(older, dest.leftCols(older.cols()), lut);
  return image;
}

AsyncGenerator<QImage> Model::Run() {
  auto source = RampSource({.sample_rate = sample_rate_,
                            .ramp_period = absl::Seconds(10),
                            .frequency_min = 100,
                            .frequency_max = 5000,
                            .pacing = SimulatedSourcePacing::kRealTime});
  auto spectra = PowerSpectrum({.sample_rate = sample_rate_,
                                .window_size = fft_window_size_,
                                .window_function = WindowFunction::kHann},
                               std::move(source));

  while (Buffer<double>* spectrum = co_await spectra) {
    AppendSpectrum(std::move(*spectrum));
    co_yield Render();
  }
}
