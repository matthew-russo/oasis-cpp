#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "channel.hpp"

using namespace channel;

TEST(ChannelTest, CanConstructSenderAndReceiver) {
  std::pair<Sender<uint32_t>, Receiver<uint32_t>> sender_receiver = mkChannel<uint32_t>();
}

TEST(ChannelTest, SenderCanSend) {
  std::pair<Sender<uint32_t>, Receiver<uint32_t>> sender_receiver = mkChannel<uint32_t>();
  Sender<uint32_t> sender = sender_receiver.first;
  sender.send(42);
}

TEST(ChannelTest, ChannelCanRecv) {
  std::pair<Sender<uint32_t>, Receiver<uint32_t>> sender_receiver = mkChannel<uint32_t>();
  Sender<uint32_t> sender = sender_receiver.first;
  Receiver<uint32_t> receiver = sender_receiver.second;
  sender.send(42);
  uint32_t t = receiver.recv();
  EXPECT_EQ(42, t);
}

// the following global atomics and functions are for the multi-threaded tests
// - ChannelRecvBlocksUntilSomethingSent
std::atomic<bool> should_send = false;
std::atomic<bool> has_sent = false;
std::atomic<bool> has_received = false;
std::atomic<uint32_t> received_value = 0;
uint32_t value_to_send = 42;

uint64_t sender_sleep_duration_ms = 10;

void delayedSend(Sender<uint32_t> sender) {
  while (!should_send.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms));
  }

  sender.send(value_to_send);
  has_sent.store(true, std::memory_order_relaxed);
}

void recv(Receiver<uint32_t> receiver) {
  uint32_t t = receiver.recv();
  has_received.store(true, std::memory_order_relaxed);
  received_value.store(t, std::memory_order_relaxed);
}

TEST(ChannelTest, ChannelRecvBlocksUntilSomethingSent) {
  std::pair<Sender<uint32_t>, Receiver<uint32_t>> sender_receiver = mkChannel<uint32_t>();
  Sender<uint32_t> sender = sender_receiver.first;
  Receiver<uint32_t> receiver = sender_receiver.second;

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
  std::pair<Sender<uint32_t>, Receiver<uint32_t>> sender_receiver = mkChannel<uint32_t>();
  Sender<uint32_t> sender = sender_receiver.first;
  Receiver<uint32_t> receiver = sender_receiver.second;
  sender.send(42);
  uint32_t t = receiver.recv();
  EXPECT_EQ(42, t);

  sender.shutdown();
  EXPECT_THROW(receiver.recv(), ChannelShutDownException);
}

// the following global atomics and functions are for the multi-threaded tests
// - ChannelShutdownWakesUpBlockedThreads
std::atomic<bool> should_shutdown = false;
std::atomic<bool> has_shutdown = false;
std::atomic<bool> has_handled_shutdown = false;

void delayedShutdown(Sender<uint32_t> sender) {
  while (!should_shutdown.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sender_sleep_duration_ms));
  }

  sender.shutdown();
  has_shutdown.store(true, std::memory_order_relaxed);
}

void recvHandleShutdown(Receiver<uint32_t> receiver) {
  try {
    uint32_t t = receiver.recv();
    throw std::runtime_error("recv should have thrown once sender shut down");
  } catch(ChannelShutDownException& e) {
    has_handled_shutdown.store(true, std::memory_order_relaxed);
  }
}

TEST(ChannelTest, ChannelShutdownWakesUpBlockedThreads) {
  std::pair<Sender<uint32_t>, Receiver<uint32_t>> sender_receiver = mkChannel<uint32_t>();
  Sender<uint32_t> sender = sender_receiver.first;
  Receiver<uint32_t> receiver = sender_receiver.second;

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
