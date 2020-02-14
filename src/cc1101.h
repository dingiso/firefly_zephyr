#pragma once

#include <device.h>
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

class Cc1101 {
 private:
  static device* spi_;
  static spi_cs_control spi_cs_cfg_;
  static spi_config spi_config_;

 public:
  void Init();
  void SetChannel(uint8_t channel) { WriteConfigurationRegister(CC_CHANNR, channel); }

  template <typename RadioPacketT>
  void Transmit(RadioPacketT& packet) {
    SetPktSize(sizeof(RadioPacketT));
    Recalibrate();
    EnterTX();
    WriteTX(packet);
  }

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

  void RfConfig();

  void Reset() { WriteStrobe(CC_SRES); }
  void EnterTX() { WriteStrobe(CC_STX); }
  void EnterRX() { WriteStrobe(CC_SRX); }
  void EnterIdle() { WriteStrobe(CC_SIDLE); }
  void FlushRxFIFO() { WriteStrobe(CC_SFRX); }
  void SetTxPower(uint8_t APwr) { WriteConfigurationRegister(CC_PATABLE, APwr); }
  void SetPktSize(uint8_t ASize) { WriteConfigurationRegister(CC_PKTLEN, ASize); }
  void Recalibrate();
};