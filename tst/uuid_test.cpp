#include <algorithm>
#include <charconv>
#include <gtest/gtest.h>
#include <sstream>
#include <string_view>

#include "uuid.hpp"
#include "time.hpp"

template <typename T>
bool vtonum(const std::string_view &view, T& value, int base = 10) {
  if (view.empty()) {
    return false;
  }

  const char* first = view.data();
  const char* last = view.data() + view.length();

  std::from_chars_result res = std::from_chars(first, last, value, base);

  if (res.ec != std::errc()) {
    return false;
  }

  if (res.ptr != last) {
    return false;
  }

  return true;
}

TEST(UuidTest, CanConstructV7Uuid) {
  oasis::uuid::Uuid v7 = oasis::uuid::Uuid::v7();
}

TEST(UuidTest, CanFormatUuid) {
  auto millisSinceEpochBefore = oasis::time::millisSinceEpoch();
  oasis::uuid::Uuid v7 = oasis::uuid::Uuid::v7();
  auto millisSinceEpochAfter = oasis::time::millisSinceEpoch();

  std::stringstream ss;
  ss << v7;
  std::string uuidStr = ss.str();
  EXPECT_EQ(36, uuidStr.length());
  EXPECT_EQ(4, std::ranges::count(uuidStr, '-'));

  /// ex: f81d4fae-7dec-11d0-a765-00a0c91e6bf6
  std::string_view first = std::string_view(uuidStr).substr(0, 8);
  // 32 bits
  uint64_t firstNum;
  EXPECT_TRUE(vtonum(first, firstNum, 16));

  std::string_view second = std::string_view(uuidStr).substr(9, 4);
  // 16 bits
  uint64_t secondNum;
  EXPECT_TRUE(vtonum(second, secondNum, 16));

  std::string_view third = std::string_view(uuidStr).substr(14, 4);
  // 16 bits
  uint64_t thirdNum;
  EXPECT_TRUE(vtonum(third, thirdNum, 16));

  std::string_view fourth = std::string_view(uuidStr).substr(19, 4);
  // 16 bits
  uint64_t fourthNum;
  EXPECT_TRUE(vtonum(fourth, fourthNum, 16));

  std::string_view fifth = std::string_view(uuidStr).substr(24, 12);
  // 48 bits
  uint64_t fifthNum;
  EXPECT_TRUE(vtonum(fifth, fifthNum, 16));

  uint64_t timestamp = firstNum | (secondNum << 32);
  uint64_t version = thirdNum & 0b1111;
  uint64_t var = fourthNum & 0b11;

  EXPECT_EQ(7, version);
  EXPECT_EQ(0b10, var);
  EXPECT_LE(millisSinceEpochBefore, timestamp);
  EXPECT_GE(millisSinceEpochAfter, timestamp);
}
