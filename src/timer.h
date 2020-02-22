#pragma once
#include <functional>
#include <kernel.h>

class Timer {
 public:
  Timer();
  ~Timer();

  // Caller is responsible for keeping action alive until it's triggered.
  // Or it can Cancel() Timer (or destruct it).
  Timer& RunDelayed(const std::function<void()>& action, uint32_t delay_ms);

  void Cancel();
 private:
  k_timer timer_;
};