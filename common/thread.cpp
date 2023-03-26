#include "thread.h"

#include <zephyr/kernel.h>

Thread::Thread(pw::Function<void()> fn) : fn_(std::move(fn)) {
  k_thread_create(&impl_, stack_, 512, [](void* arg, void*, void*) {
    (*static_cast<pw::Function<void()>*>(arg))();
  }, &fn_, nullptr, nullptr, 2, 0, K_NO_WAIT);
}