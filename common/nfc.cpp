#include "nfc.h"

#include "pw_assert/check.h"
#include "pw_bytes/span.h"
#include "pw_log/log.h"

// MFRC522 Datasheet: https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
// ISO/IEC 14443-3 standard: http://www.emutag.com/iso/14443-3.pdf
// MIFARE ISO/IEC 14443 PICC selection: https://www.nxp.com/docs/en/application-note/AN10834.pdf

using namespace nfc;

const device* Nfc::mfrc522_dev_ = DEVICE_DT_GET(DT_ALIAS(mfrc522_spi));
gpio_callback Nfc::irq_callback_data_;

const gpio_dt_spec irq_gpio_device_spec = GPIO_DT_SPEC_GET(DT_NODELABEL(nfc_irq), gpios);

const spi_cs_control Nfc::spi_cs_cfg_ = {
    .gpio = {
        .port = DEVICE_DT_GET(DT_SPI_DEV_CS_GPIOS_CTLR(DT_ALIAS(mfrc522))),
        .pin = DT_SPI_DEV_CS_GPIOS_PIN(DT_ALIAS(mfrc522)),
        .dt_flags = DT_SPI_DEV_CS_GPIOS_FLAGS(DT_ALIAS(mfrc522)),
    },
    .delay = 0,
};

const spi_config  Nfc::spi_cfg_ = {
    .frequency = 0x080000UL, // 4 MHz
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
    .slave = 0,
    .cs = &spi_cs_cfg_};

void Nfc::WriteRegister(Register reg, uint8_t value) {
  uint8_t tx[] = {(std::underlying_type<Register>::type(reg) << 1) | kWriteMask, value};
  spi_buf tx_buf = {
      .buf = tx,
      .len = 2};

  spi_buf_set tx_bufs = {
      .buffers = &tx_buf,
      .count = 1};

  spi_buf_set rx_bufs = {
      .buffers = nullptr,
      .count = 0};

  PW_CHECK_INT_EQ(spi_transceive(mfrc522_dev_, &spi_cfg_, &tx_bufs, &rx_bufs), 0);
}

uint8_t Nfc::ReadRegister(Register reg) {
  uint8_t tx[] = {(std::underlying_type<Register>::type(reg) << 1) | kReadMask, 0x00};
  uint8_t rx[] = {0, 0};

  spi_buf tx_buf = {
      .buf = tx,
      .len = 2};

  spi_buf_set tx_bufs = {
      .buffers = &tx_buf,
      .count = 1};

  spi_buf rx_buf = {
      .buf = rx,
      .len = 2};

  spi_buf_set rx_bufs = {
      .buffers = &rx_buf,
      .count = 1};

  PW_CHECK_INT_EQ(spi_transceive(mfrc522_dev_, &spi_cfg_, &tx_bufs, &rx_bufs), 0);
  return rx[1];
}

void Nfc::SetRegisterBits(Register reg, uint8_t mask) {
  WriteRegister(reg, ReadRegister(reg) | mask);
}

void Nfc::UnsetRegisterBits(Register reg, uint8_t mask) {
  WriteRegister(reg, ReadRegister(reg) & ~mask);
}

void Nfc::SendSimpleCommand(Command cmd) {
  WriteRegister(Register::FIFOLevelReg, underlying(FifoLevelRegBits::FlushBuffer));
  WriteRegister(Register::CommandReg, uint8_t(cmd));
}

void RqCallback(const device*, gpio_callback*, unsigned int pin) {
}

void Nfc::ConfigureInterrupts() {
  PW_CHECK_INT_EQ(gpio_pin_configure_dt(&irq_gpio_device_spec, GPIO_INPUT), 0);
  PW_CHECK_INT_EQ(gpio_pin_interrupt_configure_dt(&irq_gpio_device_spec, GPIO_INT_EDGE_RISING), 0);
  gpio_init_callback(&irq_callback_data_, RqCallback, BIT(irq_gpio_device_spec.pin));
  PW_CHECK_INT_EQ(gpio_add_callback(irq_gpio_device_spec.port, &irq_callback_data_), 0);
}

void Nfc::CheckWriteRead() {
  for (int v = 0; v < 8; ++v) {
    WriteRegister(Register::RFCfgReg, v << 4);
    auto read_back = ReadRegister(Register::RFCfgReg);
    PW_ASSERT(read_back == v << 4);
  }
}

void Nfc::Init() {
  ConfigureInterrupts();

  SendSimpleCommand(Command::SoftReset);
  k_sleep(K_MSEC(10));

  PW_LOG_INFO("MFRC522 version: %d", ReadRegister(Register::VersionReg));
  SendSimpleCommand(Command::Idle);

  CheckWriteRead();

  WriteRegister(Register::ComIEnReg, underlying(ComIEnRegBits::RxIEn));
  SetRegisterBits(Register::TxControlReg, underlying(TxControlRegBits::Tx1RFEn | TxControlRegBits::Tx2RFEn));
  WriteRegister(Register::RFCfgReg, underlying(RFCfgRegRegBits::Gain38db));

  // This was taken from some other library, I am not completely sure about those values.
  // Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
  WriteRegister(Register::TxASKReg, 0x40);
  // Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
  WriteRegister(Register::ModeReg, 0x3d);
}

void Nfc::CalculateCRC(pw::span<const uint8_t> data, pw::span<uint8_t> out) {
  WriteRegister(Register::DivIrqReg, underlying(~DivIrqRegBits::Set2));
  WriteRegister(Register::FIFOLevelReg, underlying(FifoLevelRegBits::FlushBuffer));
  WriteRegister(Register::CommandReg, uint8_t(Command::Idle));
  for (auto arg : data) {
    WriteRegister(Register::FIFODataReg, arg);
  }
  WriteRegister(Register::CommandReg, uint8_t(Command::CalcCRC));

  k_sleep(K_MSEC(50));

  if (any(DivIrqRegBits(ReadRegister(Register::DivIrqReg)) & DivIrqRegBits::CrcIrq)) {
    out[0] = ReadRegister(Register::CRCResultRegL);
    out[1] = ReadRegister(Register::CRCResultRegH);
  } else {
    PW_LOG_ERROR("CRC calculation failed.");
  }
  // Stop consuming data from the FIFO.
  WriteRegister(Register::CommandReg, uint8_t(Command::Idle));
}

int Nfc::Transceive(PiccCommand cmd, pw::span<const uint8_t> args, pw::span<uint8_t> response) {
  WriteRegister(Register::ComIrqReg, underlying(~ComIrqRegBits::Set1));

  switch (cmd) {
    case PiccCommand::ReqAll:
      SetRegisterBits(Register::BitFramingReg, 0x07);
      break;

    default:
      UnsetRegisterBits(Register::BitFramingReg, 0x07);
  }

  WriteRegister(Register::FIFOLevelReg, underlying(FifoLevelRegBits::FlushBuffer));
  WriteRegister(Register::FIFODataReg, underlying(cmd));
  for (auto arg : args) {
    WriteRegister(Register::FIFODataReg, arg);
  }
  WriteRegister(Register::CommandReg, uint8_t(Command::Transceive));
  SetRegisterBits(Register::BitFramingReg, 0x80);

  k_sleep(K_MSEC(50));
  UnsetRegisterBits(Register::BitFramingReg, 0x87);

  if (auto err = ReadRegister(Register::ErrorReg); err != 0) {
    Log(ErrorRegBits(err));
    return 0;
  }

  auto got_a_reply = any(ComIrqRegBits(ReadRegister(Register::ComIrqReg)) & ComIrqRegBits::RxIRq);
  if (!got_a_reply) {
    return 0;
  }

  auto reply_size = ReadRegister(Register::FIFOLevelReg);
  if (reply_size > response.size()) {
    PW_LOG_ERROR("Response (size = %d) doesn't fit into passed buffer!", reply_size);
    return 0;
  }

  for (int i = 0; i < reply_size; ++i) {
    response[i] = ReadRegister(Register::FIFODataReg);
  }

  return reply_size;
}

// Returns UID size if a card was detected, 0 otherwise.
int Nfc::SendWakeUp() {
  uint8_t rx[2];
  if (auto s = Transceive(PiccCommand::ReqAll, {}, rx); s == 2) {
    auto uid_size = (rx[0] >> 6) + 1;
    return uid_size;
  } else if (s != 0) {
    PW_LOG_ERROR("Unexpected WUPA reply size: %d", s);
    return 0;
  } else {
    return 0;
  }
}

pw::Vector<uint8_t, 10> Nfc::ReadUID() {
  uint8_t tx[] = {0x20};
  uint8_t rx[5] = {};
  pw::Vector<uint8_t, 10> result;
  for (auto cmd : {PiccCommand::SelectTag1, PiccCommand::SelectTag2, PiccCommand::SelectTag3}) {
    if (auto s = Transceive(cmd, tx, rx); s == 5) {
      auto checksum = rx[0] ^ rx[1] ^ rx[2] ^ rx[3] ^ rx[4];
      if (checksum == 0) {
        for (int i = rx[0] == 0x88 ? 1 : 0; i < 4; ++i) {
          result.push_back(rx[i]);
        }
      } else {
        PW_LOG_ERROR("AntiColl response with bad checksum: %d", checksum);
        return {};
      }
    } else if (s != 0) {
      PW_LOG_ERROR("Unexpected AntiColl reply size: %d", s);
      return {};
    } else {
      return {};
    }

    uint8_t select_tx[] = {uint8_t(cmd), 0x70, rx[0], rx[1], rx[2], rx[3], rx[4], 0, 0};
    pw::span<uint8_t> full_span = select_tx;
    CalculateCRC(full_span.first(7) , full_span.last(2));
    if (auto s = Transceive(cmd, full_span.last(8), rx); s == 3) {
      bool uid_incomplete = rx[0] & 0x04;
      if (!uid_incomplete) {
        return result;
      }
    } else {
      PW_LOG_ERROR("Unexpected Select reply size: %d", s);
      return {};
    }
  }

  return {};
}

pw::Vector<uint8_t, 10> Nfc::ReadUIDOnce() {
    if (!SendWakeUp()) {
      return {};
    }

    return ReadUID();
}

void Nfc::ReadUIDContinuously(uint32_t interval_ms, pw::Function<void(const pw::Vector<uint8_t>& uid)> callback) {
  while (true) {
    SendSimpleCommand(Command::Idle);
    k_sleep(K_MSEC(interval_ms));
    auto uid = ReadUIDOnce();
    if (!uid.empty()) {
      callback(uid);
    }
  }
}
