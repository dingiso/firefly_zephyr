#pragma once
#include <functional>
#include <kernel.h>

class Timer {
 public:
  explicit Timer(std::function<void()> action);
  Timer(const Timer& other) = delete;
  Timer(Timer&& other);
  const Timer& operator=(Timer&& other);
  ~Timer();

  void RunDelayed(uint32_t delay_ms);
  void RunEvery(uint32_t period_ms);

  void Cancel();
 private:
  k_timer timer_;
  std::function<void()> action_;
};

Timer RunDelayed(std::function<void()> action, uint32_t delay_ms);
Timer RunEvery(std::function<void()> action, uint32_t delay_ms);