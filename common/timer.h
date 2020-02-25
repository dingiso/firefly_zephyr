#pragma once
#include <functional>
#include <kernel.h>

class Timer {
 public:
  explicit Timer(std::function<void()> action);
  Timer(const Timer& other) = delete;
  Timer(Timer&& other);
  ~Timer();

  // Caller is responsible for keeping action alive until it's triggered.
  // Or it can Cancel() Timer (or destruct it).
  Timer& RunDelayed(uint32_t delay_ms);

  // Caller is responsible for keeping action alive longer than this Timer.
  // Or it can Cancel() Timer.
  Timer& RunEvery(uint32_t period_ms);

  void Cancel();
 private:
  k_timer timer_;
  std::function<void()> action_;
};

Timer RunDelayed(std::function<void()> action, uint32_t delay_ms);
Timer RunEvery(std::function<void()> action, uint32_t delay_ms);