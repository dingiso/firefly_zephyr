#include "timer.h"

namespace {
void GenericCallback(k_timer *timer) {
  auto* f = static_cast<std::function<void()>*>(k_timer_user_data_get(timer));
  (*f)();
}
}

Timer::Timer(std::function<void()> action): action_(std::move(action)) {
  k_timer_init(&timer_, GenericCallback, nullptr);
  k_timer_user_data_set(&timer_, const_cast<void*>(static_cast<const void*>(&action_)));
}

Timer::Timer(Timer&& other) {
  std::swap(timer_, other.timer_);
  std::swap(action_, other.action_);
}

Timer::~Timer() {
  Cancel();
}

Timer& Timer::RunDelayed(uint32_t delay_ms) {
  Cancel();
  k_timer_start(&timer_, delay_ms, 0);
  return *this;
}


Timer& Timer::RunEvery(uint32_t period_ms) {
  Cancel();
  k_timer_start(&timer_, period_ms, period_ms);
  return *this;
}

void Timer::Cancel() {
  k_timer_stop(&timer_);
}

Timer RunDelayed(std::function<void()> action, uint32_t delay_ms) {
  Timer t(std::move(action));
  t.RunDelayed(delay_ms);
  return t;
}

Timer RunEvery(std::function<void()> action, uint32_t period_ms) {
  Timer t(std::move(action));
  t.RunEvery(period_ms);
  return t;
}