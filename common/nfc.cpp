#include "nfc.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include "bitmap_enum.h"
#include "pw_assert/check.h"
#include "pw_bytes/span.h"
#include "pw_containers/vector.h"
#include "pw_log/log.h"

// MFRC522 Datasheet: https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
// ISO/IEC 14443-3 standard: http://www.emutag.com/iso/14443-3.pdf
// MIFARE ISO/IEC 14443 PICC selection: https://www.nxp.com/docs/en/application-note/AN10834.pdf

constexpr uint8_t kReadMask = 0b10000000;
constexpr uint8_t kWriteMask = 0b00000000;

enum class Register : uint8_t {
  // Page 0: Command and status
  CommandReg = 0x01,    // starts and stops command execution
  ComIEnReg = 0x02,     // enable and disable interrupt request control bits
  DivIEnReg = 0x03,     // enable and disable interrupt request control bits
  ComIrqReg = 0x04,     // interrupt request bits
  DivIrqReg = 0x05,     // interrupt request bits
  ErrorReg = 0x06,      // error bits showing the error status of the last command executed
  Status1Reg = 0x07,    // communication status bits
  Status2Reg = 0x08,    // receiver and transmitter status bits
  FIFODataReg = 0x09,   // input and output of 64 byte FIFO buffer
  FIFOLevelReg = 0x0A,  // number of bytes stored in the FIFO buffer
  WaterLevelReg = 0x0B, // level for FIFO underflow and overflow warning
  ControlReg = 0x0C,    // miscellaneous control registers
  BitFramingReg = 0x0D, // adjustments for bit-oriented frames
  CollReg = 0x0E,       // bit position of the first bit-collision detected on the RF interface

  // Page 1: Command
  ModeReg = 0x11,        // defines general modes for transmitting and receiving
  TxModeReg = 0x12,      // defines transmission data rate and framing
  RxModeReg = 0x13,      // defines reception data rate and framing
  TxControlReg = 0x14,   // controls the logical behavior of the antenna driver pins TX1 and TX2
  TxASKReg = 0x15,       // controls the setting of the transmission modulation
  TxSelReg = 0x16,       // selects the internal sources for the antenna driver
  RxSelReg = 0x17,       // selects internal receiver settings
  RxThresholdReg = 0x18, // selects thresholds for the bit decoder
  DemodReg = 0x19,       // defines demodulator settings
  MfTxReg = 0x1C,        // controls some MIFARE communication transmit parameters
  MfRxReg = 0x1D,        // controls some MIFARE communication receive parameters
  SerialSpeedReg = 0x1F, // selects the speed of the serial UART interface

  // Page 2: Configuration
  CRCResultRegH = 0x21, // shows the MSB and LSB values of the CRC calculation
  CRCResultRegL = 0x22,
  ModWidthReg = 0x24,   // controls the ModWidth setting?
  RFCfgReg = 0x26,      // configures the receiver gain
  GsNReg = 0x27,        // selects the conductance of the antenna driver pins TX1 and TX2 for modulation
  CWGsPReg = 0x28,      // defines the conductance of the p-driver output during periods of no modulation
  ModGsPReg = 0x29,     // defines the conductance of the p-driver output during periods of modulation
  TModeReg = 0x2A,      // defines settings for the internal timer
  TPrescalerReg = 0x2B, // the lower 8 bits of the TPrescaler value. The 4 high bits are in TModeReg.
  TReloadRegH = 0x2C,   // defines the 16-bit timer reload value
  TReloadRegL = 0x2D,
  TCounterValueRegH = 0x2E, // shows the 16-bit timer value
  TCounterValueRegL = 0x2F,

  // Page 3: Test Registers
  TestSel1Reg = 0x31,     // general test signal configuration
  TestSel2Reg = 0x32,     // general test signal configuration
  TestPinEnReg = 0x33,    // enables pin output driver on pins D1 to D7
  TestPinValueReg = 0x34, // defines the values for D1 to D7 when it is used as an I/O bus
  TestBusReg = 0x35,      // shows the status of the internal test bus
  AutoTestReg = 0x36,     // controls the digital self test
  VersionReg = 0x37,      // shows the software version
  AnalogTestReg = 0x38,   // controls the pins AUX1 and AUX2
  TestDAC1Reg = 0x39,     // defines the test value for TestDAC1
  TestDAC2Reg = 0x3A,     // defines the test value for TestDAC2
  TestADCReg = 0x3B,      // shows the value of ADC I and Q channels
};

enum class Command : uint8_t {
  Idle = 0b0000,             // no action, cancels current command execution
  Mem = 0b0001,              // stores 25 bytes into the internal buffer
  GenerateRandomID = 0b0010, // generates a 10-byte random ID number
  CalcCRC = 0b0011,          // activates the CRC coprocessor or performs a self test
  Transmit = 0b0100,         // transmits data from the FIFO buffer
  NoCmdChange = 0b0111,      // no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
  Receive = 0b1000,          // activates the receiver circuits
  Transceive = 0b1100,       // transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
  MFAuthent = 0b1110,        // performs the MIFARE standard authentication as a reader
  SoftReset = 0b1111,        // resets the MFRC522
};

enum class ComIrqRegBits : uint8_t {
  Set1 = 1 << 7,       // Write-only. Indicates that the marked bits in the ComIrqReg register are set (otherwise cleared).
  TxIRq = 1 << 6,      // Set immediately after the last bit of the transmitted data was sent out.
  RxIRq = 1 << 5,      // Receiver has detected the end of a valid data stream.
  IdleIRq = 1 << 4,    // Command termination and transition to Idle state (not set if triggered by sending Command::Idle).
  HiAlertIRq = 1 << 3, // Sticky bit set when the FIFO buffer is almost full.
  LoAlertIRq = 1 << 2, // Sticky bit set when the FIFO buffer is almost empy.
  ErrIRq = 1 << 1,     // Any error bit in the ErrorReg register is set.
  TimerIRq = 1 << 0,   // The timer decrements the timer value in register TCounterValReg to zero.
};
ENABLE_BITMASK_OPERATORS(ComIrqRegBits)

void Log(ComIrqRegBits bits) {
  PW_LOG_INFO("ComIrqReg: %d (Set1 = %d, TxIRq = %d, RxIRq = %d, IdleIRq = %d, HiAlertIRq = %d, LoAlertIRq = %d, ErrIRq = %d, TimerIRq = %d)",
              underlying(bits),
              any(bits & ComIrqRegBits::Set1),
              any(bits & ComIrqRegBits::TxIRq),
              any(bits & ComIrqRegBits::RxIRq),
              any(bits & ComIrqRegBits::IdleIRq),
              any(bits & ComIrqRegBits::HiAlertIRq),
              any(bits & ComIrqRegBits::LoAlertIRq),
              any(bits & ComIrqRegBits::ErrIRq),
              any(bits & ComIrqRegBits::TimerIRq));
}

enum class DivIrqRegBits {
  Set2 = 1 << 7,       // Write-only. Indicates that the marked bits in the DivIrqReg register are set (otherwise cleared).
  MfinActIRq = 1 << 4, // MFIN is active
  CrcIrq = 1 << 2,     // the CalcCRC command is active and all data is processed
};
ENABLE_BITMASK_OPERATORS(DivIrqRegBits)

enum class Status1RegBits : uint8_t {
  CRCOk = 1 << 6,    // For data transmission and reception, the CRCOk bit is undefined: use the ErrorReg register’s CRCErr bit.
  CRCReady = 1 << 5, // Only valid for the CRC coprocessor calculation using the CalcCRC command.
  IRq = 1 << 4,      // Indicates if any interrupt source requests attention with respect to the setting of the interrupt enable
                     // bits: see the ComIEnReg and DivIEnReg registers.
  TRunning = 1 << 3, // MFRC522’s timer unit is running, i.e. the timer will decrement the
                     // TCounterValReg register with the next timer clock.
  HiAlert = 1 << 1,  // Currently, FIFO buffer is almost full.
  LoAlert = 1 << 0,  // Currently, FIFO buffer is almost empty.
};
ENABLE_BITMASK_OPERATORS(Status1RegBits);

void Log(Status1RegBits bits) {
  PW_LOG_INFO("Status1Reg: %d (CRCOk = %d, CRCReady = %d, IRq = %d, TRunning = %d, HiAlert = %d, LoAlert = %d)",
              underlying(bits),
              any(bits & Status1RegBits::CRCOk),
              any(bits & Status1RegBits::CRCReady),
              any(bits & Status1RegBits::IRq),
              any(bits & Status1RegBits::TRunning),
              any(bits & Status1RegBits::HiAlert),
              any(bits & Status1RegBits::LoAlert));
}

enum class RFCfgRegRegBits : uint8_t {
  Gain18db = 0b010 << 4,
  Gain23db = 0b011 << 4,
  Gain33db = 0b100 << 4,
  Gain38db = 0b101 << 4,
  Gain43db = 0b110 << 4,
  Gain48db = 0b111 << 4,
};
ENABLE_BITMASK_OPERATORS(RFCfgRegRegBits)

enum class FifoLevelRegBits : uint8_t {
  FlushBuffer = 1 << 7, //  Immediately clears the internal FIFO buffer’s read and write pointer and ErrorReg register’s BufferOvfl bit.
  // Lower bits represent the number of bytes stored in the FIFO buffer.
};
ENABLE_BITMASK_OPERATORS(FifoLevelRegBits)

enum class ComIEnRegBits : uint8_t {
  IRqInv = 1 << 7,     // Invert the signal on the IRQ pit (make it active low)
  TxIEn = 1 << 6,      // Allows the transmitter interrupt request (TxIRq bit) to be propagated to IRQ pin
  RxIEn = 1 << 5,      // Allows the transmitter interrupt request (RxIRq bit) to be propagated to IRQ pin
  IdleIEn = 1 << 4,    // Allows the idle interrupt request (IdleIRq bit) to be propagated to IRQ pin
  HiAlertIEn = 1 << 3, // Allows the high alert interrupt request (HiAlertIRq bit) to be propagated to IRQ pin
  LoAlertIEn = 1 << 2, // Allows the low alert interrupt request (LoAlertIRq bit) to be propagated to IRQ pin
  ErrIEn = 1 << 1,     // Allows the error interrupt request (ErrIRq bit) to be propagated to IRQ pin
  TimerIEn = 1 << 0,   // Allows the timer interrupt request (TimerIRq bit)  to be propagated to IRQ pin
};
ENABLE_BITMASK_OPERATORS(ComIEnRegBits)

enum class ErrorRegBits : uint8_t {
  WrErr = 1 << 7,       // Data is written into the FIFO buffer by the host during the MFAuthent
                        // command or if data is written into the FIFO buffer by the host during the
                        // time between sending the last bit on the RF interface and receiving the
                        // last bit on the RF interface.
  TempErr = 1 << 6,     // Internal temperature sensor detects overheating, in which case the
                        // antenna drivers are automatically switched off.
  BufferOvfl = 1 << 4,  // Indicates a FIFO overflow.
  CollErr = 1 << 3,     // A bit-collision is detected
                        // * cleared automatically at receiver start-up phase
                        // * only valid during the bitwise anticollision at 106 kBd
  CRCErr = 1 << 2,      // The RxModeReg register’s RxCRCEn bit is set and the CRC calculation fails
                        // * automatically cleared to logic 0 during receiver start-up phase
  ParityErr = 1 << 1,   // Parity check failed
                        // * automatically cleared during receiver start-up phase
                        // * only valid for ISO/IEC 14443 A/MIFARE communication at 106 kBd
  ProtocolErr = 1 << 0, // Set to logic 1 if the SOF is incorrect
                        // * automatically cleared during receiver start-up phase
                        // * bit is only valid for 106 kBd.
};
ENABLE_BITMASK_OPERATORS(ErrorRegBits)
void Log(ErrorRegBits bits) {
  PW_LOG_INFO("ErrorReg: %d (WrErr = %d, TempErr = %d, BufferOvfl = %d, CollErr = %d, CRCErr = %d, ParityErr = %d, ProtocolErr = %d)",
              underlying(bits),
              any(bits & ErrorRegBits::WrErr),
              any(bits & ErrorRegBits::TempErr),
              any(bits & ErrorRegBits::BufferOvfl),
              any(bits & ErrorRegBits::CollErr),
              any(bits & ErrorRegBits::CRCErr),
              any(bits & ErrorRegBits::ParityErr),
              any(bits & ErrorRegBits::ProtocolErr));
}

enum class TxControlRegBits {
  // There are other bits in this register, but they are not used in this driver.
  // Ones below are 0 by default, need to be 1 to enable antenna.
  Tx2RFEn = 1 << 1, // Output signal on pin TX2 delivers the 13.56 MHz energy carrier
                    // modulated by the transmission data.
  Tx1RFEn = 1 << 0, // Output signal on pin TX1 delivers the 13.56 MHz energy carrier
                    // modulated by the transmission data.
};
ENABLE_BITMASK_OPERATORS(TxControlRegBits)

enum class PiccCommand : uint8_t {
  ReqIdl = 0x26,    // find the antenna area does not enter hibernation
  ReqAll = 0x52,    // find all the cards antenna area
  SelectTag1 = 0x93, // election card cascade level 1
  SelectTag2 = 0x95, // election card cascade level 2
  SelectTag3 = 0x97, // election card cascade level 3
  Authent1A = 0x60, // authentication key A
  Authent1B = 0x61, // authentication key B
  Read = 0x30,      // Read Block
  Write = 0xA0,     // write block
  Decrement = 0xC0, // debit
  Increment = 0xC1, // recharge
  Restore = 0xC2,   // transfer block data to the buffer
  Transfer = 0xB0,  // save the data in the buffer
  Halt = 0x50       // Sleep
};
ENABLE_BITMASK_OPERATORS(PiccCommand)

static const device* mfrc522_dev = DEVICE_DT_GET(DT_ALIAS(mfrc522_spi));

const gpio_dt_spec irq_gpio_device_spec = GPIO_DT_SPEC_GET(DT_NODELABEL(nfc_irq), gpios);
static gpio_callback irq_callback_data;

const gpio_dt_spec beeper_gpio_device_spec = GPIO_DT_SPEC_GET(DT_NODELABEL(rx_beeper), gpios);

static const spi_cs_control spi_cs_cfg = {
    .gpio = {
        .port = DEVICE_DT_GET(DT_SPI_DEV_CS_GPIOS_CTLR(DT_ALIAS(mfrc522))),
        .pin = DT_SPI_DEV_CS_GPIOS_PIN(DT_ALIAS(mfrc522)),
        .dt_flags = DT_SPI_DEV_CS_GPIOS_FLAGS(DT_ALIAS(mfrc522)),
    },
    .delay = 0,
};

static const spi_config spi_cfg = {
    .frequency = 0x080000UL, // 4 MHz
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
    .slave = 0,
    .cs = &spi_cs_cfg};

void WriteRegister(Register reg, uint8_t value) {
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

  PW_CHECK_INT_EQ(spi_transceive(mfrc522_dev, &spi_cfg, &tx_bufs, &rx_bufs), 0);
}

uint8_t ReadRegister(Register reg) {
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

  PW_CHECK_INT_EQ(spi_transceive(mfrc522_dev, &spi_cfg, &tx_bufs, &rx_bufs), 0);
  return rx[1];
}

void SetRegisterBits(Register reg, uint8_t mask) {
  WriteRegister(reg, ReadRegister(reg) | mask);
}

void UnsetRegisterBits(Register reg, uint8_t mask) {
  WriteRegister(reg, ReadRegister(reg) & ~mask);
}

void SendSimpleCommand(Command cmd) {
  WriteRegister(Register::FIFOLevelReg, underlying(FifoLevelRegBits::FlushBuffer));
  WriteRegister(Register::CommandReg, uint8_t(cmd));
}

void RqCallback(const device*, gpio_callback*, unsigned int pin) {
}

void ConfigureInterrupts() {
  PW_CHECK_INT_EQ(gpio_pin_configure_dt(&irq_gpio_device_spec, GPIO_INPUT), 0);
  PW_CHECK_INT_EQ(gpio_pin_interrupt_configure_dt(&irq_gpio_device_spec, GPIO_INT_EDGE_RISING), 0);
  gpio_init_callback(&irq_callback_data, RqCallback, BIT(irq_gpio_device_spec.pin));
  PW_CHECK_INT_EQ(gpio_add_callback(irq_gpio_device_spec.port, &irq_callback_data), 0);

  PW_CHECK_INT_EQ(gpio_pin_configure_dt(&beeper_gpio_device_spec, GPIO_OUTPUT), 0);
}

void CheckWriteRead() {
  for (int v = 0; v < 8; ++v) {
    WriteRegister(Register::RFCfgReg, v << 4);
    auto read_back = ReadRegister(Register::RFCfgReg);
    PW_ASSERT(read_back == v << 4);
  }
}

void Init() {
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

void CalculateCRC(pw::span<const uint8_t> data, pw::span<uint8_t> out) {
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

// Returns the number of bytes in the response.
int Transceive(PiccCommand cmd, pw::span<const uint8_t> args, pw::span<uint8_t> response) {
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
int SendWakeUp() {
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

pw::Vector<uint8_t, 10> ReadUID(uint8_t uid_size) {
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

void Nfc::RunTests() {
  Init();
  while (true) {
    SendSimpleCommand(Command::Idle);
    k_sleep(K_MSEC(400));

    int uid_size = SendWakeUp();
    if (!uid_size) {
      continue;
    }

    auto uid = ReadUID(uid_size);
    if (uid.empty()) {
      continue;
    }

    if (uid.size() == 4) {
      PW_LOG_INFO("4 byte UID: %02x %02x %02x %02x", uid[0], uid[1], uid[2], uid[3]);
    } else if (uid.size() == 7) {
      PW_LOG_INFO("7 byte UID: %02x %02x %02x %02x %02x %02x %02x", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
    } else {
      PW_LOG_WARN("Unsupported UID length: %d", uid.size());
      continue;
    }

    gpio_pin_set_dt(&beeper_gpio_device_spec, 1);
    k_sleep(K_MSEC(100));
    gpio_pin_set_dt(&beeper_gpio_device_spec, 0);
  }
}