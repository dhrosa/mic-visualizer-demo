#include "qaudio_source_coro.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <stop_token>
#include <thread>

void RunApp(std::stop_token stop_token) {
  std::string arg0 = "fake_program_name";
  char* argv[2] = {arg0.data(), nullptr};
  int argc = 1;
  QCoreApplication app(argc, argv);
  auto quitter = std::stop_callback(stop_token, [&] { app.quit(); });
  app.exec();
}

class QAudioSourceCoroTest : public testing::Test {
 private:
  std::jthread app_thread_ =
      std::jthread([](std::stop_token token) { RunApp(token); });
};

TEST_F(QAudioSourceCoroTest, Empty) { QAudioSourceCoro source; }

TEST_F(QAudioSourceCoroTest, SingleFrame) {
  QAudioSourceCoro source;
  auto frames = source.Frames();

  auto iter = frames.begin().Wait();
  EXPECT_FALSE(iter == frames.end());

  Buffer<std::int16_t> frame = std::move(*iter);
  EXPECT_EQ(frame.size(), 5);
}
