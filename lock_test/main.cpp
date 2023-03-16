#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include <vector>
#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>

#include "printk_event_handler.h"

#include "gtest/gtest.h"
#include "test.pwpb.h"

TEST(BasicTest, Sum) {
  ASSERT_EQ(2 + 2, 4);
}

TEST(ProtoTest, Equality) {
  static_assert(pw::protobuf::IsTriviallyComparable<lock_test::pwpb::Customer::Message>());
  lock_test::pwpb::Customer::Message a, b;
  a.age = 5;
  a.status = lock_test::pwpb::Customer::Status::ACTIVE;
  a.name = "Alexey";

  b.age = 5;
  b.status = lock_test::pwpb::Customer::Status::ACTIVE;
  b.name = "Alexey";

  ASSERT_EQ(a, b);
}

int main() {
  PrintkEventHandler handler;
  pw::unit_test::RegisterEventHandler(&handler);

  int num_failures = RUN_ALL_TESTS();
  if (!num_failures) {
    printk("All tests passed!\n");
  }
  while(true) k_sleep(K_MSEC(1000));
}
