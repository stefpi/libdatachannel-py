#pragma once

#include <rtc/mediahandler.hpp>
#include <rtc/rtp.hpp>

#include <cstdint>
#include <mutex>
#include <optional>

namespace rtc {

struct RrStats {
  uint32_t ssrc;
  uint8_t fractionLost;
  uint32_t packetsLost;
  uint32_t highestSeqNo;
  uint32_t jitter;
  uint32_t lsr;
  uint32_t dlsr;
};

class RrHandler final : public MediaHandler {
 public:
  RrHandler();

  void incoming(message_vector& messages, const message_callback& send) override;
  std::optional<RrStats> getStats();

 private:
  std::mutex mMutex;
  std::optional<RrStats> mStats;
};

}  // namespace rtc
