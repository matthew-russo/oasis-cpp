#ifndef OASIS_UUID_H
#define OASIS_UUID_H

#include <cstdint>
#include <iostream>
#include <random>

#include "time.hpp"

namespace oasis {
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

  Uuid(uint64_t lo, uint64_t hi) : lo(lo), hi(hi) {};

  // allow our overload to print internals
  friend std::ostream& operator<<(std::ostream&, const Uuid&);

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
    static const uint64_t varClearMask = 0b11LLU;
    // bits 64 - 65, set to 0b10
    static const uint64_t varMask = 0b10LLU;

    std::random_device rd;
    std::mt19937_64 mt(rd());

    uint64_t lo = time::millisSinceEpoch();
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

/// The formal definition of the UUID string representation is provided by the following ABNF [RFC5234]:
/// UUID     = 4hexOctet "-"
///            2hexOctet "-"
///            2hexOctet "-"
///            2hexOctet "-"
///            6hexOctet
/// hexOctet = HEXDIG HEXDIG
/// DIGIT    = %x30-39
/// HEXDIG   = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"
///
/// ex: f81d4fae-7dec-11d0-a765-00a0c91e6bf6
std::ostream& operator<<(std::ostream& os, const Uuid& uuid)
{
  auto first = uuid.lo & 0x0000'0000'FFFF'FFFFLLU;
  auto second = (uuid.lo & 0x0000'FFFF'0000'0000LLU) >> 32;
  auto third = (uuid.lo & 0xFFFF'0000'0000'0000LLU) >> 48;
  auto fourth = uuid.hi & 0x0000'0000'0000'FFFFLLU;
  auto fifth = (uuid.hi & 0xFFFF'FFFF'FFFF'0000LLU) >> 16;

  return os << std::hex
    << std::setw(8) << std::setfill('0') << first << "-"
    << std::setw(4) << std::setfill('0') << second << "-"
    << std::setw(4) << std::setfill('0') << third << "-" 
    << std::setw(4) << std::setfill('0') << fourth << "-"
    << std::setw(12) << std::setfill('0') << fifth;
}
}; // namespace uuid
}; // namespace oasis

#endif // OASIS_UUID_H
