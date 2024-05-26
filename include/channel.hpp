#ifndef OASIS_CHANNEL_CPP_H
#define OASIS_CHANNEL_CPP_H

#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace channel {
struct ChannelShutDownException : public std::runtime_error {
public:
  ChannelShutDownException()
      : std::runtime_error("Channel has been shut down") {}

  const char *what() const throw() { return "Channel has been shut down"; }
};

template <typename T> class Channel {
private:
  std::deque<T> queue;
  std::mutex lock;
  std::condition_variable cond;
  bool is_shutdown = false;

  std::optional<T> tryRecv() {
    std::unique_lock<std::mutex> guard(lock);
    if (queue.empty()) {
      return {};
    } else {
      T val = queue.front();
      queue.pop_front();

      return val;
    }
  }

  T recv() {
    std::unique_lock<std::mutex> guard(lock);
    cond.wait(guard, [this]() { return !queue.empty() || is_shutdown; });

    if (is_shutdown) {
      throw ChannelShutDownException();
    }

    T val = queue.front();
    queue.pop_front();

    return val;
  }

  void send(T val) {
    std::lock_guard<std::mutex> guard(lock);
    queue.push_back(std::move(val));
    cond.notify_one();
  }

  void shutdown() {
    std::lock_guard<std::mutex> guard(lock);
    is_shutdown = true;
    cond.notify_all();
  }

  template <typename U> friend class Receiver;
  template <typename U> friend class Sender;
};

template <typename T> class Receiver {
public:
  Receiver() {}
  Receiver(Channel<T> *c) : chan(c) {}

  std::optional<T> tryRecv() { return chan->tryRecv(); }

  T recv() { return chan->recv(); }

private:
  Channel<T> *chan;
};

template <typename T> class Sender {
public:
  Sender() {}
  Sender(Channel<T> *c) : chan(c) {}

  void send(T val) { return chan->send(val); }

  void shutdown() { return chan->shutdown(); }

private:
  Channel<T> *chan;
};

template <typename T> std::pair<Sender<T>, Receiver<T>> mkChannel() {
  Channel<T> *chan = new Channel<T>();
  return std::pair(Sender<T>(chan), Receiver<T>(chan));
}
}; // namespace channel

#endif
