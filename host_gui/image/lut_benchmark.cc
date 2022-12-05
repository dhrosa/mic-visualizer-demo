#include <benchmark/benchmark.h>

#include <Eigen/Core>
#include <QImage>
#include <random>
#include <ranges>

#include "lut.h"
#include "qimage_eigen.h"

static void BM_Lut(benchmark::State& state) {
  const std::size_t width = state.range(0);
  const std::size_t height = state.range(1);

  QImage image(width, height, QImage::Format_ARGB32);
  std::uint32_t lut[256];
  std::mt19937 rng{std::random_device{}()};
  std::uniform_int_distribution<std::uint32_t> distribution;
  std::ranges::generate(lut, [&] { return distribution(rng); });

  auto source = Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic>::Random(
      height, width);
  auto dest = EigenView(image);
  for (auto _ : state) {
    LutMap(source, dest, lut, -0.5, 0.5);
  }
  state.SetItemsProcessed(width * height * state.iterations());
}

std::vector<std::int64_t> Sizes() {
  return std::vector<std::int64_t>({250, 500, 1000, 2000, 4000});
}

BENCHMARK(BM_Lut)
    ->ArgNames({"width", "height"})
    ->ArgsProduct({Sizes(), Sizes()});
