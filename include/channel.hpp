#ifndef OASIS_CHANNEL_H
#define OASIS_CHANNEL_H

#include <cassert>
#include <condition_variable>
#include <deque>
#include <expected>
#include <mutex>
#include <optional>

namespace channel {
enum class ChannelError {
  Shutdown,
};

template <typename T> class Channel {
private:
  std::deque<T> queue;
  std::mutex lock;
  std::condition_variable cond;
  bool is_shutdown = false;

  std::expected<std::optional<T>, ChannelError> tryRecv() {
    std::unique_lock<std::mutex> guard(lock);

    if (is_shutdown) {
      return std::unexpected(ChannelError::Shutdown);
    }

    if (queue.empty()) {
      return {};
    } else {
      T val = queue.front();
      queue.pop_front();
      return val;
    }
  }

  std::expected<T, ChannelError> recv() {
    std::unique_lock<std::mutex> guard(lock);
    cond.wait(guard, [this]() { return !queue.empty() || is_shutdown; });

    if (is_shutdown) {
      return std::unexpected(ChannelError::Shutdown);
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

  std::expected<std::optional<T>, ChannelError> tryRecv() { return chan->tryRecv(); }

  std::expected<T, ChannelError> recv() { return chan->recv(); }

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
