#pragma once

#include <device.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <logging/log.h>
#include <zephyr.h>
#include "cc1101_constants.h"
#include "cc1101_rf_settings.h"

// Uses SPI_1 instance. So devicetree should contain something like
// &spi1 {
//   status = "okay";
//   sck-pin = <6>;
//   mosi-pin = <8>;
//   miso-pin = <7>;
//   cs-gpios = <&gpio0 5 0>;
// };

// Datasheet: http://www.ti.com/lit/ds/symlink/cc1101.pdf

class Cc1101 {
 private:
  const static device* spi_;
  static spi_cs_control spi_cs_cfg_;
  static spi_config spi_config_;
  static k_sem gd_ready_;
  static gpio_callback gdo0_callback_data_;

 public:
  void Init();
  void SetChannel(uint8_t channel) { WriteConfigurationRegister(CC_CHANNR, channel); }

  template <typename RadioPacketT>
  void Transmit(RadioPacketT& packet) {
    SetPacketSize(sizeof(RadioPacketT));
    Recalibrate();
    EnterTX();
    WriteTX(packet);
    k_sem_take(&gd_ready_, K_FOREVER);
  }

  template<typename RadioPacketT>
  bool Receive(uint32_t timeout_ms, RadioPacketT* result) {
    LOG_MODULE_DECLARE();

    SetPacketSize(sizeof(RadioPacketT));
    Recalibrate();
    FlushRxFIFO();
    EnterRX();
    if (k_sem_take(&gd_ready_, K_MSEC(timeout_ms)) == 0) {
      return ReadFifo(result);
    } else {
      EnterIdle();
      return false;
    }
  }

  void EnterPwrDown() { WriteStrobe(CC_SPWD); }

 private:
  // Sends a single-byte instruction to the CC1101.
  // See documentation of instructions in datasheet, p.32,
  // 10.4 Command Strobes
  // If status is provided, status byte will be written into it.
  void WriteStrobe(uint8_t instruction, uint8_t* status = nullptr);

  // Sets a configuration register to a provided value.
  // See detailed description of available registers in datasheet, p.66
  // 29 Configuration Registers and Table 45: SPI Address Space.
  // If statuses is provided (must be a 2-byte array), status bytes will be written into it.
  void WriteConfigurationRegister(uint8_t reg, uint8_t value, uint8_t* statuses = nullptr);

  // Reads a configuration or status register.
  // If status is provided, status byte will be written into it.
  uint8_t ReadRegister(uint8_t reg, uint8_t* status = nullptr);

  template <typename RadioPacketT>
  void WriteTX(RadioPacketT& packet) {
    uint8_t tx_flags = CC_FIFO | CC_WRITE_FLAG | CC_BURST_FLAG;

    spi_buf tx_bufs[2];

    tx_bufs[0].buf = &tx_flags;
    tx_bufs[0].len = 1;

    tx_bufs[1].buf = static_cast<void*>(&packet);
    tx_bufs[1].len = sizeof(RadioPacketT);

    spi_buf_set tx_bufs_set = {
        .buffers = tx_bufs,
        .count = 2};

    auto r = spi_write(spi_, &spi_config_, &tx_bufs_set);
    if (r != 0) {
      LOG_MODULE_DECLARE();
      LOG_ERR("WriteTX fail: %d", r);
    }
  }

  template<typename RadioPacketT>
  bool ReadFifo(RadioPacketT* result) {
    LOG_MODULE_DECLARE();
    uint8_t status = 0;
    uint8_t b = ReadRegister(CC_PKTSTATUS, &status);
    if (!(b & 0x80)) {
      LOG_WRN("Weird, no data, packet status = %d, read register status =%d", b, status);
      return false;
    } else {
      LOG_DBG("CRC OK, packet status = %d, read register status =%d", b, status);
    }
    uint8_t tx = CC_FIFO | CC_READ_FLAG | CC_BURST_FLAG;
    uint8_t rx[sizeof(RadioPacketT) + 3];

    spi_buf tx_buf = {
        .buf = &tx,
        .len = 1};

    spi_buf_set tx_bufs = {
        .buffers = &tx_buf,
        .count = 1};

    spi_buf rx_buf = {
        .buf = rx,
        .len = sizeof(RadioPacketT) + 3};

    spi_buf_set rx_bufs = {
        .buffers = &rx_buf,
        .count = 1};

    auto r = spi_transceive(spi_, &spi_config_, &tx_bufs, &rx_bufs);
    if (r != 0) {
      LOG_MODULE_DECLARE();
      LOG_ERR("ReadFifo fail: %d", r);
    }

    memcpy(result, rx + 1, sizeof(RadioPacketT));
    return true;
  }

  static void Gdo0Callback(const device *dev, gpio_callback *cb, gpio_port_pins_t pins);

  void RfConfig();

  void Reset() { WriteStrobe(CC_SRES); }
  void EnterTX() { WriteStrobe(CC_STX); }
  void EnterRX() { WriteStrobe(CC_SRX); }
  void EnterIdle() { WriteStrobe(CC_SIDLE); }
  void FlushRxFIFO() { WriteStrobe(CC_SFRX); }
  void SetTxPower(uint8_t APwr) { WriteConfigurationRegister(CC_PATABLE, APwr); }
  void Recalibrate();

 public:
  void SetPacketSize(uint8_t ASize) { WriteConfigurationRegister(CC_PKTLEN, ASize); }
  uint8_t GetPacketSize() { return ReadRegister(CC_PKTLEN); }
};