#include <gtest/gtest.h>
#include <sys/event.h>

#include "os/kqueue.hpp"

void testHandler(oasis::os::KqueuePollerHandle* poller, struct kevent kevent, void* ctx) {}

TEST(KqueueTest, CanConstructKqueuePoller) {
  oasis::os::KqueuePoller poller;
}

TEST(KqueueTest, CanSpawnKqueuePoller) {
  oasis::os::KqueuePoller poller;
  poller.spawn();
}

TEST(KqueueTest, CanJoinSpawnedKqueuePoller) {
  oasis::os::KqueuePoller poller;
  poller.spawn();
  poller.join();
}

TEST(KqueueTest, CanAddHandlerToIdleKqueuePoller) {
  oasis::os::KqueuePoller poller;

  oasis::os::KqueuePair pair(1, EVFILT_TIMER);
  oasis::os::KqueueHandler handler(nullptr, testHandler);

  poller.addHandler(pair, 1000, handler);
}

TEST(KqueueTest, CanRemoveHandlerFromIdleKqueuePoller) {
  oasis::os::KqueuePoller poller;

  oasis::os::KqueuePair pair(1, EVFILT_TIMER);
  oasis::os::KqueueHandler handler(nullptr, testHandler);

  poller.addHandler(pair, 1000, handler);
  poller.removeHandler(pair);
}

TEST(KqueueTest, CanSpawnKqueuePollerWithHandle) {
  oasis::os::KqueuePoller poller;

  oasis::os::KqueuePair pair(1, EVFILT_TIMER);
  oasis::os::KqueueHandler handler(nullptr, testHandler);

  poller.addHandler(pair, 1000, handler);
  poller.spawn();
}

TEST(KqueueTest, CanAddHandlerToSpawnedKqueuePoller) {
  oasis::os::KqueuePoller poller;

  oasis::os::KqueuePair pair(1, EVFILT_TIMER);
  oasis::os::KqueueHandler handler(nullptr, testHandler);

  poller.spawn();
  poller.addHandler(pair, 1000, handler);
}

TEST(KqueueTest, CanRemoveHandlerFromSpawnedKqueuePoller) {
  oasis::os::KqueuePoller poller;

  oasis::os::KqueuePair pair(1, EVFILT_TIMER);
  oasis::os::KqueueHandler handler(nullptr, testHandler);

  poller.addHandler(pair, 1000, handler);
  poller.spawn();
  poller.removeHandler(pair);
}
