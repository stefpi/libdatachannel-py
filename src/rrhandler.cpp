#include "rrhandler.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace rtc {
namespace {

constexpr uint8_t RTCP_RR_PAYLOAD_TYPE = 201;

}  // namespace

RrHandler::RrHandler() = default;

void RrHandler::incoming(message_vector& messages,
                         [[maybe_unused]] const message_callback& send) {
  for (const auto& message : messages) {
    if (!message || message->type != Message::Control) {
      continue;
    }

    size_t offset = 0;
    while ((sizeof(RtcpHeader) + offset) <= message->size()) {
      const auto* header =
          reinterpret_cast<const RtcpHeader*>(message->data() + offset);
      const size_t packetSize = header->lengthInBytes();
      if (packetSize < sizeof(RtcpHeader) ||
          packetSize > message->size() - offset) {
        break;
      }

      if (header->payloadType() == RTCP_RR_PAYLOAD_TYPE &&
          header->reportCount() > 0 &&
          packetSize >= RtcpRr::SizeWithReportBlocks(header->reportCount())) {
        const auto* rr = reinterpret_cast<const RtcpRr*>(header);
        const auto* reportBlock = rr->getReportBlock(0);
        const uint32_t loss = ntohl(reportBlock->_fractionLostAndPacketsLost);
        RrStats stats;
        stats.ssrc = reportBlock->getSSRC();
        stats.fractionLost = static_cast<uint8_t>(loss >> 24);
        stats.packetsLost = loss & 0x00FFFFFF;
        stats.highestSeqNo = reportBlock->extendedHighestSeqNo();
        stats.jitter = reportBlock->jitter();
        stats.lsr = ntohl(reportBlock->_lastReport);
        stats.dlsr = reportBlock->delaySinceSR();

        std::lock_guard lock(mMutex);
        mStats = stats;
      }

      offset += packetSize;
    }
  }
}

std::optional<RrStats> RrHandler::getStats() {
  std::lock_guard lock(mMutex);
  return mStats;
}

}  // namespace rtc
