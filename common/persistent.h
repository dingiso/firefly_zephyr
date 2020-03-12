#pragma once

#include "eeprom.h"

template <typename T> class Persistent {
 public:
  Persistent(uint32_t magic): magic_(magic) {}

  void LoadOrInit(const T& default_value) {
    eeprom::EnablePower();
    struct Loaded {
      uint32_t magic;
      T value;
    };

    const auto loaded = eeprom::Read<Loaded>(0);
    if (loaded.magic == magic_) {
      value_ = loaded.value;
    } else {
      value_ = default_value;
      Save();
    }
  }

  T& value() {
    return value_;
  }

  void Save() {
    eeprom::Write(*this, 0);
  }

 private:
  const uint32_t magic_;
  T value_;
};
