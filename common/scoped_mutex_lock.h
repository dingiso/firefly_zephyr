#pragma once

#include <kernel.h>

// Helper to automatically release mutex on scope end
class ScopedMutexLock {
public:
  ScopedMutexLock(k_mutex& m, int32_t timeout = K_FOREVER) : m_(m) {
    locked_ = k_mutex_lock(&m_, timeout) == 0;
  }

  ~ScopedMutexLock() {
    if (locked()) {
      k_mutex_unlock(&m_);
    }
  }

  bool locked() const { return locked_; };

private:
  k_mutex& m_;
  bool locked_;
};
