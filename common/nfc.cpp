#include "nfc.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#include "bitmap_enum.h"
#include "pw_assert/check.h"
#include "pw_bytes/span.h"
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
              uint8_t(bits),
              any(bits & ComIrqRegBits::Set1),
              any(bits & ComIrqRegBits::TxIRq),
              any(bits & ComIrqRegBits::RxIRq),
              any(bits & ComIrqRegBits::IdleIRq),
              any(bits & ComIrqRegBits::HiAlertIRq),
              any(bits & ComIrqRegBits::LoAlertIRq),
              any(bits & ComIrqRegBits::ErrIRq),
              any(bits & ComIrqRegBits::TimerIRq));
}

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
              uint8_t(bits),
              any(bits & Status1RegBits::CRCOk),
              any(bits & Status1RegBits::CRCReady),
              any(bits & Status1RegBits::IRq),
              any(bits & Status1RegBits::TRunning),
              any(bits & Status1RegBits::HiAlert),
              any(bits & Status1RegBits::LoAlert));
}

enum class FifoLevelRegBits : uint8_t {
  FlushBuffer = 1 << 7,    //  Immediately clears the internal FIFO buffer’s read and write pointer and ErrorReg register’s BufferOvfl bit.
  // Lower bits represent the number of bytes stored in the FIFO buffer.
};

static const device* mfrc522_dev = DEVICE_DT_GET(DT_ALIAS(mfrc522_spi));

const gpio_dt_spec irq_gpio_device_spec = GPIO_DT_SPEC_GET(DT_NODELABEL(nfc_irq), gpios);
static gpio_callback irq_callback_data;

static const spi_cs_control spi_cs_cfg = {
    .gpio = {
        .port = DEVICE_DT_GET(DT_SPI_DEV_CS_GPIOS_CTLR(DT_ALIAS(mfrc522))),
        .pin = DT_SPI_DEV_CS_GPIOS_PIN(DT_ALIAS(mfrc522)),
        .dt_flags = DT_SPI_DEV_CS_GPIOS_FLAGS(DT_ALIAS(mfrc522)),
    },
    .delay = 0,
};

static const spi_config spi_cfg = {
    .frequency = 0x400000UL, // 4 MHz
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

void SendCommand(Command cmd, pw::span<const uint8_t> arguments = {}) {
  WriteRegister(Register::FIFOLevelReg, uint8_t(FifoLevelRegBits::FlushBuffer));
  for (auto arg : arguments) {
    WriteRegister(Register::FIFODataReg, arg);
  }
  WriteRegister(Register::CommandReg, uint8_t(cmd));
  if (cmd == Command::Transceive) {
    SetRegisterBits(Register::BitFramingReg, 0x87);
  }
}

void RqCallback(const device*, gpio_callback*, unsigned int pin) {
  PW_LOG_INFO("--------------------------------------------------------------------- RqCallback %d", pin);
}

void ConfigureInterrupts() {
  auto ret = gpio_pin_configure_dt(&irq_gpio_device_spec, GPIO_INPUT);
  if (ret != 0) LOG_ERR("Failed to configure button pin: %d", ret);

  ret = gpio_pin_interrupt_configure_dt(&irq_gpio_device_spec, GPIO_INT_EDGE_RISING);
  if (ret != 0) LOG_ERR("Failed to configure interrupt on pin: %d", ret);

  gpio_init_callback(&irq_callback_data, RqCallback, BIT(irq_gpio_device_spec.pin));
  gpio_add_callback(irq_gpio_device_spec.port, &irq_callback_data);
}

void CheckWriteRead() {
  for (int v = 0; v < 8; ++v) {
    WriteRegister(Register::RFCfgReg, v << 4);
    auto read_back = ReadRegister(Register::RFCfgReg);
    PW_ASSERT(read_back == v << 4);
  }
}

void OverflowFIFO() {
  for (int i = 0; i < 70; ++i) {
    WriteRegister(Register::FIFODataReg, 0x33);
    PW_LOG_INFO("Fifo level: %d", ReadRegister(Register::FIFOLevelReg));
    PW_LOG_INFO("Status1: %x", ReadRegister(Register::Status1Reg));
    PW_LOG_INFO("Status2: %x", ReadRegister(Register::Status2Reg));
    k_sleep(K_MSEC(20));
  }
}

void Nfc::RunTests() {
  ConfigureInterrupts();

  SendCommand(Command::SoftReset);
  k_sleep(K_MSEC(10));

  PW_LOG_INFO("MFRC522 version: %d", ReadRegister(Register::VersionReg));
  SendCommand(Command::Idle);

  CheckWriteRead();

  WriteRegister(Register::ComIEnReg, 0b01111110); // HiAlert interrupt
  SetRegisterBits(Register::TxControlReg, 0x03); // Enable antenna
  WriteRegister(Register::RFCfgReg, 0x70); // Maximal gain

  // Reset baud rates
  WriteRegister(Register::TxModeReg, 0x00);
  WriteRegister(Register::RxModeReg, 0x00);

  // Reset ModWidthReg
  WriteRegister(Register::ModWidthReg, 0x26);

  // Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
  WriteRegister(Register::TxASKReg, 0x40);

  // Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
  WriteRegister(Register::ModeReg, 0x3d);

  while (true) {
    SendCommand(Command::Idle);
    WriteRegister(Register::ComIrqReg, 0b01111111);

    k_sleep(K_MSEC(5));

    uint8_t picc_command_wupa[] = {0x52};
    SendCommand(Command::Transceive, picc_command_wupa);
    k_sleep(K_MSEC(10));
    for (int i = 0; i < 1; ++i) {
      auto irq = ReadRegister(Register::ComIrqReg); // Read the ComIrqReg register
      if (any(ComIrqRegBits(irq) & ComIrqRegBits::RxIRq)) {
        PW_LOG_INFO("Something happenned in response to WUPA!");
        Log(ComIrqRegBits(irq));
        Log(Status1RegBits(ReadRegister(Register::Status1Reg)));
      }
      if (auto er = ReadRegister(Register::ErrorReg); er != 0) {
        PW_LOG_INFO("ErrorReg: %x", er);
      }
      k_sleep(K_MSEC(10));
    }

    WriteRegister(Register::BitFramingReg, ReadRegister(Register::BitFramingReg) & (~0x80));

    k_sleep(K_MSEC(50));
    if (auto b = ReadRegister(Register::FIFOLevelReg); b > 0) {
      PW_LOG_INFO("Fifo level: %d", b);
      for (int i = 0; i < b; ++i) {
        PW_LOG_INFO("Fifo data: %x", ReadRegister(Register::FIFODataReg));
      }
    }
    k_sleep(K_MSEC(200));
  }
}