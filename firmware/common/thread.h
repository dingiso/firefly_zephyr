#pragma once

#include "pw_function/function.h"

#include <zephyr/kernel.h>

class Thread {
 public:
  explicit Thread(pw::Function<void()> fn);

 private:
  pw::Function<void()> fn_;
  k_thread impl_;
  K_KERNEL_STACK_MEMBER(stack_, 1024);
};