#pragma once

#include "zephyr/drivers/spi.h"

#include "pw_bytes/span.h"
#include "pw_assert/assert.h"

enum class RegisterKind {
  R = 0,
  RW = 1,
  W = 2,
};

template<typename AddressType, typename CommandType>
class GenericDevice {
 public:
  static_assert(sizeof(AddressType) == 1, "AddressType must be 1 byte long");

  GenericDevice(const device* spi_dev, const spi_config* spi_config) : spi_dev_(spi_dev), spi_config_(spi_config) {
    PW_ASSERT(device_is_ready(spi_config->cs->gpio.port));
    PW_ASSERT(device_is_ready(spi_dev));
  };

  uint8_t ReadRegisterRaw(uint8_t address) {
    AddressType a;
    a.address = address;
    a.read = true;

    const spi_buf tx_bufs[] = {
      {.buf = &a, .len = 1},
    };
    const spi_buf_set tx = {
      .buffers = tx_bufs,
      .count = 1,
    };

    uint8_t result = 0;
    const spi_buf rx_bufs[] = {
      {.buf = nullptr, .len = 1},
      {.buf = &result, .len = 1},
    };
    const spi_buf_set rx = {
      .buffers = rx_bufs,
      .count = 2,
    };

    auto err = spi_transceive(spi_dev_, spi_config_, &tx, &rx);
    PW_ASSERT(err == 0);

    return result;
  }

  void WriteRegisterRaw(uint8_t address, uint8_t value) {
    AddressType a;
    a.address = address;
    a.read = false;

    const spi_buf tx_bufs[] = {
      {.buf = &address, .len = 1},
      {.buf = &value, .len = 1},
    };
    const spi_buf_set tx = {
      .buffers = tx_bufs,
      .count = 2,
    };

    auto err = spi_transceive(spi_dev_, spi_config_, &tx, nullptr);
    PW_ASSERT(err == 0);
  }

  void ModifyRegisterRaw(uint8_t address, uint8_t mask, uint8_t value) {
    auto current_value = ReadRegisterRaw(address);
    current_value &= ~mask;
    current_value |= (value & mask);
    WriteRegisterRaw(address, current_value);
  }

  template<typename T>
  T ReadRegister() {
    static_assert(sizeof(T) == 1, "T must be 1 byte long");
    static_assert(T::kind == RegisterKind::R || T::kind == RegisterKind::RW, "T must be a readable register");
    uint8_t result_raw = ReadRegisterRaw(T::address);
    return *reinterpret_cast<T*>(&result_raw);
  }

  template<typename T>
  void WriteRegister(T value) {
    static_assert(sizeof(T) == 1, "T must be 1 byte long");
    static_assert(T::kind == RegisterKind::W || T::kind == RegisterKind::RW, "T must be a writeable register");
    WriteRegisterRaw(T::address, *reinterpret_cast<uint8_t*>(&value));
  }

  template<typename T, typename F>
  void ModifyRegister(F f) {
    static_assert(sizeof(T) == 1, "T must be 1 byte long");
    static_assert(T::kind == RegisterKind::W || T::kind == RegisterKind::RW, "T must be a writeable register");
    auto r = ReadRegister<T>();
    f(r);
    WriteRegister(r);
  }

  void SendCommand(CommandType cmd) {
    static_assert(sizeof(CommandType) == 1, "CommandType must be 1 byte long");

    const spi_buf tx_bufs[] = {
      {.buf = &cmd, .len = 1},
    };
    const spi_buf_set tx = {
      .buffers = tx_bufs,
      .count = 1,
    };

    auto err = spi_transceive(spi_dev_, spi_config_, &tx, nullptr);
    PW_ASSERT(err == 0);
  }

  template<typename T, typename F>
  class RegisterModifier {
   public:
    RegisterModifier(GenericDevice& device, F f) :
      device_(device),
      original_value_(device_.ReadRegister<T>()) {
        auto new_value = original_value_;
        f(new_value);
        device_.WriteRegister(new_value);
    }

    ~RegisterModifier() {
      device_.WriteRegister(original_value_);
    }
   private:
    GenericDevice& device_;
    T original_value_;
  };

  template<typename T, typename F>
  RegisterModifier<T, F> MakeRegisterModifier(F f) {
    return {*this, std::move(f)};
  }

 private:
  const device* spi_dev_ = nullptr;
  const spi_config* spi_config_ = nullptr;
};