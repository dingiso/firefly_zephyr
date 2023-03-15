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

TEST(BasicTest, Sum) {
  ASSERT_EQ(2 + 2, 4);
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
