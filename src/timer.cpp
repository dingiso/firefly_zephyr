#include "timer.h"

namespace {
void GenericCallback(k_timer *timer) {
  auto* f = static_cast<std::function<void()>*>(k_timer_user_data_get(timer));
  (*f)();
}
}

Timer::Timer() {
  k_timer_init(&timer_, GenericCallback, nullptr);
}

Timer::~Timer() {
  Cancel();
}

Timer& Timer::RunDelayed(const std::function<void()>& action, uint32_t delay_ms) {
  k_timer_user_data_set(&timer_, const_cast<void*>(static_cast<const void*>(&action)));
  k_timer_start(&timer_, delay_ms, 0);
  return *this;
}

void Timer::Cancel() {
  k_timer_stop(&timer_);
}
