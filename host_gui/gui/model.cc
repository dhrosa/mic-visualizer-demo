#include "model.h"

#include <absl/time/time.h>

#include <ranges>

#include "audio/source.h"
#include "audio/spectrum.h"
#include "image/lut.h"
#include "image/qimage_eigen.h"

Model::Model()
    : frequency_bins_(FrequencyBins(fft_window_size_, sample_rate_)),
      data_(768, frequency_bins_.size()) {}

Generator<absl::AnyInvocable<void(QImage&) &&>> Model::Run() {
  auto spectra = PowerSpectrum(sample_rate_, fft_window_size_,
                               SimulatedSource(absl::Milliseconds(10)));
  for (auto&& spectrum : std::move(spectra)) {
    std::ranges::for_each(spectrum, [&](auto& v) {
      v = std::log2(v + 1);
      min_value_ = std::min(v, min_value_);
      max_value_ = std::max(v, max_value_);
    });
    data_.AppendColumn(spectrum);
    co_yield [&](QImage& image) {
      auto lut = active_colormap_->entries;
      auto dest = EigenView(image);

      auto newer = data_.Newer();
      auto older = data_.Older();

      LutMap(newer, dest.rightCols(newer.cols()), lut, min_value_, max_value_);
      LutMap(older, dest.leftCols(older.cols()), lut, min_value_, max_value_);
    };
  }
}
