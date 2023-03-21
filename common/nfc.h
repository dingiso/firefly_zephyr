#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include "pw_function/function.h"
#include "pw_containers/vector.h"
#include "pw_span/span.h"

#include "nfc_definitions.h"

class Nfc {
 public:
  void Init();

  pw::Vector<uint8_t, 10> ReadUIDOnce();
  [[noreturn]] void ReadUIDContinuously(uint32_t interval_ms, pw::Function<void(const pw::Vector<uint8_t>& uid)> callback);

 private:
  void WriteRegister(nfc::Register reg, uint8_t value);
  uint8_t ReadRegister(nfc::Register reg);
  void SetRegisterBits(nfc::Register reg, uint8_t mask);
  void UnsetRegisterBits(nfc::Register reg, uint8_t mask);

  void SendSimpleCommand(nfc::Command cmd);

  // Returns the number of bytes in the response.
  int Transceive(nfc::PiccCommand cmd, pw::span<const uint8_t> args, pw::span<uint8_t> response);
  void CalculateCRC(pw::span<const uint8_t> data, pw::span<uint8_t> out);

  // Returns UID size if a card was detected, 0 otherwise.
  int SendWakeUp();

  // Empty vector means failed attempt to read UID.
  pw::Vector<uint8_t, 10> ReadUID();

  void ConfigureInterrupts();
  void CheckWriteRead();

  const static device* mfrc522_dev_;
  const static spi_cs_control spi_cs_cfg_;
  const static spi_config spi_cfg_;
  static gpio_callback irq_callback_data_;
};
