#ifndef OASIS_TIME_H
#define OASIS_TIME_H

#include <cstdint>
#include <chrono>

namespace oasis {
namespace time {
uint64_t millisSinceEpoch() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
}; // namespace time
}; // namespace oasis

#endif // OASIS_TIME_H
