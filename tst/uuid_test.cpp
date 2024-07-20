#include <gtest/gtest.h>

#include "uuid.hpp"

using namespace uuid;

TEST(UuidTest, CanConstructV7Uuid) {
  Uuid v7 = Uuid::v7();
}
