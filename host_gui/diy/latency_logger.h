#pragma once

#include <absl/log/log.h>
#include <absl/time/clock.h>

class LatencyLogger {
 public:
  [[nodiscard]] LatencyLogger(const std::string& label) : label_(label) {}

  ~LatencyLogger() {
    const absl::Duration latency = absl::Now() - start_;
    LOG(INFO) << label_ << ": " << latency;
  }

 private:
  const std::string label_;
  const absl::Time start_ = absl::Now();
};
