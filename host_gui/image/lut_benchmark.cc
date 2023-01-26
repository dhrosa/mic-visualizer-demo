#include <benchmark/benchmark.h>

#include <Eigen/Core>
#include <QImage>
#include <random>
#include <ranges>

#include "lut.h"
#include "qimage_eigen.h"

auto RandomLut() {
  std::mt19937 rng{std::random_device{}()};
  std::uniform_int_distribution<std::uint32_t> distribution;

  std::array<std::uint32_t, 256> lut;
  std::ranges::generate(lut, [&] { return distribution(rng); });
  return lut;
}

auto RandomDoubles(std::size_t n) {
  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
  ;

  std::vector<double> values(n);
  std::ranges::generate(values, [&] { return distribution(rng); });
  return values;
}

static void BM_ToIndexed(benchmark::State& state) {
  const std::size_t n = state.range(0);
  const std::vector<double> values = RandomDoubles(n);
  std::vector<std::uint8_t> indexed(n);

  for (auto _ : state) {
    ToIndexed(values, indexed, 0, 1);
  }
  state.SetItemsProcessed(n * state.iterations());
}

static void BM_LutMap(benchmark::State& state) {
  static std::array<std::uint32_t, 256> lut = RandomLut();
  const std::size_t width = state.range(0);
  const std::size_t height = state.range(1);
  auto source =
      Eigen::Array<std::uint8_t, Eigen::Dynamic, Eigen::Dynamic>::Random(height,
                                                                         width)
          .eval();
  QImage image(width, height, QImage::Format_ARGB32);
  auto dest = EigenView(image);
  for (auto _ : state) {
    LutMap(source, dest, lut);
  }
  state.SetItemsProcessed(width * height * state.iterations());
}

std::vector<std::int64_t> Sizes() {
  return std::vector<std::int64_t>({250, 1000, 4000});
}

BENCHMARK(BM_ToIndexed)
    ->Arg(256)
    ->Arg(512)
    ->Arg(1024)
    ->Arg(2048)
    ->Arg(4096)
    ->Arg(8192);

BENCHMARK(BM_LutMap)
    ->ArgNames({"width", "height"})
    ->ArgsProduct({Sizes(), Sizes()});
