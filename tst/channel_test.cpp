#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "sync/channel.hpp"

TEST(ChannelTest, CanConstructSenderAndReceiver) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
}

TEST(ChannelTest, SenderCanSend) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Sender<uint32_t> sender = sender_receiver.first;
  sender.send(42);
}

TEST(ChannelTest, ChannelCanRecv) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Sender<uint32_t> sender = sender_receiver.first;
  oasis::sync::channel::Receiver<uint32_t> receiver = sender_receiver.second;
  sender.send(42);
  std::expected<uint32_t, oasis::sync::channel::ChannelError> t = receiver.recv();
  EXPECT_TRUE(t.has_value());
  EXPECT_EQ(42, t.value());
}

// the following global atomics and functions are for the multi-threaded tests
// - ChannelRecvBlocksUntilSomethingSent
std::atomic<bool> should_send = false;
std::atomic<bool> has_sent = false;
std::atomic<bool> has_received = false;
std::atomic<uint32_t> received_value = 0;
uint32_t value_to_send = 42;

uint64_t sender_sleep_duration_ms = 10;

void delayedSend(oasis::sync::channel::Sender<uint32_t> sender) {
  while (!should_send.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms));
  }

  sender.send(value_to_send);
  has_sent.store(true, std::memory_order_relaxed);
}

void recv(oasis::sync::channel::Receiver<uint32_t> receiver) {
  std::expected<uint32_t, oasis::sync::channel::ChannelError> t_res = receiver.recv();
  // ensure we didn't get an error
  uint32_t t = t_res.value();
  has_received.store(true, std::memory_order_relaxed);
  received_value.store(t, std::memory_order_relaxed);
}

TEST(ChannelTest, ChannelRecvBlocksUntilSomethingSent) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Sender<uint32_t> sender = sender_receiver.first;
  oasis::sync::channel::Receiver<uint32_t> receiver = sender_receiver.second;

  std::thread delayed_sender_thread(delayedSend, sender);
  std::thread blocking_recv_thread(recv, receiver);

  // sleep for an arbitrarily long amount of time
  std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms * 3));

  // validate we haven't sent or received anything
  EXPECT_FALSE(should_send.load(std::memory_order_relaxed));
  EXPECT_FALSE(has_sent.load(std::memory_order_relaxed));
  EXPECT_FALSE(has_received.load(std::memory_order_relaxed));
  EXPECT_EQ(0, received_value.load(std::memory_order_relaxed));

  // unblock the sender
  should_send.store(true, std::memory_order_relaxed);

  // sleep for enough time to allow the sender to send and
  // receiver to receive
  std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms * 2));

  EXPECT_TRUE(has_sent.load(std::memory_order_relaxed));
  EXPECT_TRUE(has_received.load(std::memory_order_relaxed));
  EXPECT_EQ(value_to_send, received_value.load(std::memory_order_relaxed));

  delayed_sender_thread.join();
  blocking_recv_thread.join();
}

TEST(ChannelTest, ChannelCanShutdown) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Sender<uint32_t> sender = sender_receiver.first;
  oasis::sync::channel::Receiver<uint32_t> receiver = sender_receiver.second;
  sender.send(42);
  std::expected<uint32_t, oasis::sync::channel::ChannelError> t_with_value = receiver.recv();
  EXPECT_EQ(true, t_with_value.has_value());
  EXPECT_EQ(42, t_with_value.value());

  sender.shutdown();
  std::expected<uint32_t, oasis::sync::channel::ChannelError> t_with_err = receiver.recv();
  EXPECT_EQ(false, t_with_err.has_value());
  EXPECT_EQ(oasis::sync::channel::ChannelError::Shutdown, t_with_err.error());
}

// the following global atomics and functions are for the multi-threaded tests
// - ChannelShutdownWakesUpBlockedThreads
std::atomic<bool> should_shutdown = false;
std::atomic<bool> has_shutdown = false;
std::atomic<bool> has_handled_shutdown = false;

void delayedShutdown(oasis::sync::channel::Sender<uint32_t> sender) {
  while (!should_shutdown.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms));
  }

  sender.shutdown();
  has_shutdown.store(true, std::memory_order_relaxed);
}

void recvHandleShutdown(oasis::sync::channel::Receiver<uint32_t> receiver) {
  std::expected<uint32_t, oasis::sync::channel::ChannelError> t = receiver.recv();
  if (!t.has_value()) {
    has_handled_shutdown.store(true, std::memory_order_relaxed);
  }
}

TEST(ChannelTest, ChannelShutdownWakesUpBlockedThreads) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Sender<uint32_t> sender = sender_receiver.first;
  oasis::sync::channel::Receiver<uint32_t> receiver = sender_receiver.second;

  std::thread delayed_shutdown_thread(delayedShutdown, sender);
  std::thread blocking_recv_thread(recvHandleShutdown, receiver);

  // sleep for an arbitrarily long amount of time
  std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms * 3));

  // validate we haven't shutdown or received anything
  EXPECT_FALSE(should_shutdown.load(std::memory_order_relaxed));
  EXPECT_FALSE(has_shutdown.load(std::memory_order_relaxed));
  EXPECT_FALSE(has_handled_shutdown.load(std::memory_order_relaxed));

  // unblock the shutdown
  should_shutdown.store(true, std::memory_order_relaxed);

  // sleep for enough time to allow the shutdown to occur
  std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms * 2));

  EXPECT_TRUE(has_shutdown.load(std::memory_order_relaxed));
  EXPECT_TRUE(has_handled_shutdown.load(std::memory_order_relaxed));

  delayed_shutdown_thread.join();
  blocking_recv_thread.join();
}

TEST(ChannelTest, ChannelTryRecvReturnsEmptyOptionalIfEmpty) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Receiver<uint32_t> receiver = sender_receiver.second;
  std::expected<std::optional<uint32_t>, oasis::sync::channel::ChannelError> t_res = receiver.tryRecv();
  EXPECT_EQ(true, t_res.has_value());
  std::optional<uint32_t> t = t_res.value();
  EXPECT_EQ(false, t.has_value());
}

TEST(ChannelTest, ChannelTryRecvReturnsFrontIfNonEmpty) {
  std::pair<
    oasis::sync::channel::Sender<uint32_t>,
    oasis::sync::channel::Receiver<uint32_t>
  > sender_receiver = oasis::sync::channel::mkChannel<uint32_t>();
  oasis::sync::channel::Sender<uint32_t> sender = sender_receiver.first;
  oasis::sync::channel::Receiver<uint32_t> receiver = sender_receiver.second;
  sender.send(42);
  std::expected<std::optional<uint32_t>, oasis::sync::channel::ChannelError> t_res = receiver.tryRecv();
  EXPECT_EQ(true, t_res.has_value());
  std::optional<uint32_t> t = t_res.value();
  EXPECT_EQ(true, t.has_value());
  EXPECT_EQ(42, t.value());
}
