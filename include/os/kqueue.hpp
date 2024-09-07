#ifndef OASIS_OS_KQUEUE_H
#define OASIS_OS_KQUEUE_H

#include <atomic>
#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <sys/event.h>
#include <thread>
#include <unordered_map>

namespace oasis {
namespace os {

struct KqueuePair {
  uintptr_t ident;
  int16_t filter;

  KqueuePair(uintptr_t ident, int16_t filter) : ident(ident), filter(filter) {}
  
  inline bool operator==(const KqueuePair& rhs) const {
    return this->ident == rhs.ident && this->filter == rhs.filter;
  }

  inline bool operator!=(const KqueuePair& rhs) const {
    return !operator==(rhs);
  }
};

}; // namespace os
}; // namespace oasis

// we need to specialize std::hash before we use it in the unordered map below
template <>
struct std::hash< oasis::os::KqueuePair > {
  std::size_t operator()(const oasis::os::KqueuePair& pair) const {
    size_t h1 = hash<uintptr_t>()(pair.ident);
    size_t h2 = hash<int16_t>()(pair.filter);
    return h1 ^ (h2 << 1);
  }
};

namespace oasis {
namespace os {

enum class KqueuePollerError {

};

class KqueuePollerHandle;

using KqueueHandlerFn = std::function<
  void(KqueuePollerHandle*, struct kevent, void* /* opaque context */)
>;

class KqueueHandler {
  void* ctx;
  KqueueHandlerFn handler;

public:
  KqueueHandler(void* ctx, KqueueHandlerFn handler) : ctx(ctx), handler(handler) {}

  void handle(KqueuePollerHandle* poller, struct kevent kevent) {
    this->handler(poller, kevent, ctx);
  }
};

class KqueuePoller {
  // give our handle internal access so it can perform raw
  // locks/unlocks
  friend class KqueuePollerHandler;

  // 10ms (10 * 1000 nanos per micro * 1000 micros per milli)
  static constexpr struct timespec CTRL_TIMEOUT = { 0, 10 * 1000 * 1000 } ;
  // 1ms (1 * 1000 nanos per micro * 1000 micros per milli)
  static constexpr struct timespec TIMEOUT = { 0, 1 * 1000 * 1000 };

  std::shared_mutex handlersGuard;
  std::unordered_map< KqueuePair, KqueueHandler > handlers;

  int kqfd;

  std::mutex pollingThreadGuard;
  std::optional< std::thread > pollingThread;

  std::atomic< bool > shutdownSignal = false;

  void addHandlerRaw(KqueuePair pair, intptr_t data, KqueueHandler handler);
  void removeHandlerRaw(KqueuePair pair);
  std::expected< void, KqueuePollerError > mainLoop();

public:
  KqueuePoller();
  ~KqueuePoller();
  void spawn();
  bool isSpawned();
  void join();
  void addHandler(KqueuePair pair, intptr_t data, KqueueHandler handler);
  void removeHandler(KqueuePair pair);
};

class KqueuePollerHandle {
  KqueuePoller* poller;

public:
  KqueuePollerHandle(KqueuePoller* poller) : poller(poller) {}

  void addHandler(KqueuePair pair, intptr_t data, KqueueHandler handler) {
    this->poller->addHandler(pair, data, handler);
  }

  void removeHandler(KqueuePair pair) {
    this->poller->removeHandler(pair);
  }
};

inline void KqueuePoller::addHandlerRaw(KqueuePair pair, intptr_t data, KqueueHandler handler) {
  struct kevent event;
  EV_SET(
    &event,
    pair.ident,
    pair.filter,
    EV_ADD | EV_ENABLE,
    0,
    data,
    NULL
  );

  int ret = kevent(this->kqfd, &event, 1, NULL, 0, &this->CTRL_TIMEOUT);

  if (ret == -1) {
    // TODO [matthew-russo 09-02-2024] handle error
    throw std::runtime_error("[KqueuePoller] failed to add new event to kqueue via kevent syscall");
  }

  auto iterAndInserted = this->handlers.insert({ pair, handler });
  if (!iterAndInserted.second) {
    // TODO [matthew-russo 09-02-2024] handle error
    throw std::runtime_error("[KqueuePoller] duplicate handler");
  }
}

inline void KqueuePoller::removeHandlerRaw(KqueuePair pair) {
  if (this->handlers.contains(pair)) {
    struct kevent event;
    EV_SET(
      &event,
      pair.ident,
      pair.filter,
      EV_DELETE,
      0,
      0,
      NULL
    );
  
    int ret = kevent(this->kqfd, &event, 1, NULL, 0, &this->CTRL_TIMEOUT);

    if (ret == -1) {
      // TODO [matthew-russo 09-02-2024] handle error
      throw std::runtime_error("[KqueuePoller] failed to add new event to kqueue via kevent syscall");
    }

    assert(this->handlers.erase(pair) == 1);
  }
}

inline std::expected< void, KqueuePollerError > KqueuePoller::mainLoop() {
  const uint32_t maxEvents = 1024;
  struct kevent events[maxEvents];

  while (!this->shutdownSignal.load(std::memory_order_relaxed)) {
    uint32_t numEvents = kevent(this->kqfd, NULL, 0, events, maxEvents, &this->TIMEOUT);

    if (numEvents == -1) {
      // TODO [matthew-russo 09-02-2024] handle error
      throw std::runtime_error("[KqueuePoller] failed to wait on new events via kevent syscall");
    } else if (numEvents == 0) {
      // do nothing, we just timed out
    } else {
      for (uint32_t idx = 0; idx < numEvents; idx++) {
        struct kevent event = events[idx];
        KqueuePair pair(event.ident, event.filter);
        KqueuePollerHandle selfHandle(this);
        std::shared_lock guard(this->handlersGuard);
        this->handlers.at(pair).handle(&selfHandle, event);
      }
    }
  }

  return std::expected< void, KqueuePollerError >{};
}

inline KqueuePoller::KqueuePoller() {
  this->kqfd = kqueue();
  if (this->kqfd == -1) {
    // TODO [matthew-russo 09-02-2024] handle error
    throw std::runtime_error("[KqueuePoller] failed to construct kqueue (syscall)");
  }
}

inline KqueuePoller::~KqueuePoller() {
  this->join();
}

inline void KqueuePoller::spawn() {
  std::unique_lock guard(this->pollingThreadGuard);

  if (this->pollingThread.has_value()) {
    // TODO [matthew-russo 09-02-2024] handle error
    throw std::runtime_error("[KqueuePoller] trying to spawn pollingThread but it has already been spawned");
  }

  this->pollingThread = std::thread(&KqueuePoller::mainLoop, this);
}

inline bool KqueuePoller::isSpawned() {
  std::unique_lock guard(this->pollingThreadGuard);
  return this->pollingThread.has_value();
}

inline void KqueuePoller::join() {
  std::unique_lock guard(this->pollingThreadGuard);

  if (this->pollingThread.has_value()) {
    this->shutdownSignal.store(true, std::memory_order_relaxed);
    this->pollingThread.value().join();
    this->pollingThread = std::nullopt;
    this->shutdownSignal.store(false, std::memory_order_relaxed);
  }
}

inline void KqueuePoller::addHandler(KqueuePair pair, intptr_t data, KqueueHandler handler) {
  std::unique_lock guard(this->handlersGuard);
  this->addHandlerRaw(pair, data, handler);
}

inline void KqueuePoller::removeHandler(KqueuePair pair) {
  std::unique_lock guard(this->handlersGuard);
  this->removeHandlerRaw(pair);
}

}; // namespace os
}; // namespace oasis

#endif // OASIS_OS_TCP_H
