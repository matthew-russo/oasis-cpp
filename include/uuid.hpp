#ifndef OASIS_UUID_H
#define OASIS_UUID_H

#include <chrono>
#include <cstdint>
#include <random>

namespace uuid {

/// Universally Unique IDentifiers
///
/// https://datatracker.ietf.org/doc/html/rfc9562
class Uuid {
private:
  uint64_t lo;
  uint64_t hi;

  // bits 0 - 48
  static const uint64_t timestampMask = 0x0000'FFFF'FFFF'FFFFLLU;

  static uint64_t millisSinceEpoch() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  }

  Uuid(uint64_t lo, uint64_t hi) : lo(lo), hi(hi) {};

public:
  //  0                   1                   2                   3
  //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                           unix_ts_ms                          |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |          unix_ts_ms           |  ver  |       rand_a          |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |var|                        rand_b                             |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                            rand_b                             |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  static Uuid v7() {
    // bits 48 - 51, set to 0b0111 (7)
    static const uint64_t versionMask = 0b0111LLU << 48;
    // bits 52 - 63
    static const uint64_t randAMask = 0xFFF0'0000'0000'0000LLU;
    // bits 64 - 65, set to 0b10
    static const uint64_t varClearMask = 0b00LLU;
    // bits 64 - 65, set to 0b10
    static const uint64_t varMask = 0b10LLU;

    std::random_device rd;
    std::mt19937_64 mt(rd());

    uint64_t lo = Uuid::millisSinceEpoch();
    // clear the upper 16 bits of timestamp so we can use it as lo
    lo &= timestampMask;
    lo |= versionMask;

    uint64_t randA = mt();
    randA &= randAMask;

    lo |= randA;
    
    uint64_t hi = mt();
    hi &= ~varClearMask;
    hi |= varMask;

    return Uuid(lo, hi);
  }
};

}; // namespace uuid

#endif
