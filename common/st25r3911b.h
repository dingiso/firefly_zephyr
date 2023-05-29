#pragma once

#include "generic_device.h"

// Driver for ST25R3911B NFC reader.
// Datasheet is available at https://www.st.com/resource/en/datasheet/st25r3911b.pdf.

namespace st25r3911b {

struct [[gnu::packed]] Address {
  uint8_t address : 6;
  bool read : 1;
  uint8_t unused : 1 = 0;
};

enum class BitRate : uint8_t {
  Bpsk_106 = 0b000,
  Bpsk_212 = 0b001,
  Bpsk_424 = 0b010,
  Bpsk_848 = 0b011,
  Bpsk_1695 = 0b100,
  Bpsk_3390 = 0b101,
  Bpsk_6780 = 0b110,
};

struct [[gnu::packed]] IoConfigurationRegister1 {
  static constexpr uint8_t address = 0x00;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class OutClockFrequency : uint8_t {
    MHz_3_39 = 0b00,
    MHz_6_78 = 0b01,
    MHz_13_56 = 0b10,
    Disabled = 0b11,
  };

  enum class OscillatorFrequency : uint8_t {
    MHz_13_56 = 0,
    MHz_27_12 = 1,
  };

  enum class FifoTransmitWaterLevel : uint8_t{
    Bytes32 = 0,
    Bytes16 = 1,
  };

  enum class FifoReceiveWaterLevel : uint8_t {
    Bytes64 = 0,
    Bytes80 = 1,
  };

  enum class OutputDriver : uint8_t{
    RF_0 = 0,
    RF_1 = 1,
  };

  bool lf_clk_off : 1;
  OutClockFrequency out_cl: 2;
  OscillatorFrequency osc : 1;
  FifoTransmitWaterLevel fifo_lt : 1;
  FifoReceiveWaterLevel fifo_lr : 1;
  OutputDriver rfo : 1;   // Which driver to use in the single driving mode
  bool single_driver : 1; // Use single driving instead of differential
};

struct [[gnu::packed]] IoConfigurationRegister2 {
  static constexpr uint8_t address = 0x01;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class PowerSupply : uint8_t {
    v5 = 0,
    v3_3 = 1,
  };

  bool slow_up: 1;
  uint8_t : 1;
  bool io_18: 1;
  bool miso_pd1: 1;
  bool miso_pd2: 1;
  uint8_t : 1;
  bool vspd_off: 1;
  PowerSupply sup: 1;
};

struct [[gnu::packed]] OperationControlRegister {
  static constexpr uint8_t address = 0x02;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t : 2;
  bool wake_up : 1;
  bool tx_en : 1;
  bool rx_manual: 1;
  bool rx_chn : 1;
  bool rx_en : 1;
  bool en : 1;
};

struct [[gnu::packed]] ModeDefinitionRegister {
  static constexpr uint8_t address = 0x03;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class OperationMode : uint8_t {
    // Initiator modes
    NfcIp1ActiveCommunication = 0b0000,
    Iso14443A = 0b0001,
    Iso14443B = 0b0010,
    FeliCa = 0b0011,
    NfcForumType1TagTopaz = 0b0100,
    SubcarrierStream = 0b1110,
    BpskStream = 0b1111,

    // Target modes
    NfcIp1ActiveCommunicationBitRateDetection = 0b0000,
    NfcIp1ActiveCommunicationNormal = 0b0001,
  };

  // Automatic start Response RF Collision Avoidance sequence
  bool nfc_ar: 1;
  uint8_t : 2;
  OperationMode om : 4;
  bool targ: 1;
};

struct [[gnu::packed]] BitRateDefinitionRegister {
  static constexpr uint8_t address = 0x04;
  static constexpr RegisterKind kind = RegisterKind::RW;

  BitRate rx_rate : 4;
  BitRate tx_rate : 4;
};

struct [[gnu::packed]] Iso14443ASettingsRegister {
  static constexpr uint8_t address = 0x05;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool antcl: 1;
  uint8_t p_len: 4;
  bool nfc_f0: 1;
  bool no_rx_par: 1;
  bool no_tx_par: 1;
};

struct [[gnu::packed]] Iso14443BSettingsRegister1 {
  static constexpr uint8_t address = 0x06;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t raw : 8;
};

struct [[gnu::packed]] Iso14443BSettingsRegister2 {
  static constexpr uint8_t address = 0x07;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t raw : 8;
};

struct [[gnu::packed]] StreamModeDefinitionRegister {
  static constexpr uint8_t address = 0x08;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t raw : 8;
};

struct [[gnu::packed]] AuxiliaryDefinitionRegister {
  static constexpr uint8_t address = 0x09;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t nfc_n : 2;
  bool rx_tol : 1;
  bool ook_hr: 1;
  bool en_fd: 1;
  bool tr_am: 1;
  bool crc_2_fifo: 1;
  bool no_crc_rx: 1;
};

struct [[gnu::packed]] ReceiverConfigurationRegister1 {
  static constexpr uint8_t address = 0x0A;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t z12k : 1;
  uint8_t h80 : 1;
  uint8_t h200 : 1;
  uint8_t lp: 3;
  uint8_t amd_sel: 1;
  uint8_t ch_sel: 1;
};

struct [[gnu::packed]] ReceiverConfigurationRegister2 {
  static constexpr uint8_t address = 0x0B;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t pmix_cl : 1;
  uint8_t sqm_dyn : 1;
  uint8_t agc_alg : 1;
  uint8_t agc_m : 1;
  uint8_t agc_en : 1;
  uint8_t lf_en : 1;
  uint8_t lf_op : 1;
  uint8_t rx_lp : 1;
};

struct [[gnu::packed]] ReceiverConfigurationRegister3 {
  static constexpr uint8_t address = 0x0C;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t rg_nfc : 1;
  uint8_t lim : 1;
  uint8_t rg1_pm : 3;
  uint8_t rg1_am : 3;
};

struct [[gnu::packed]] ReceiverConfigurationRegister4 {
  static constexpr uint8_t address = 0x0D;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t rg2_pm : 4;
  uint8_t rg2_am : 4;
};

struct [[gnu::packed]] MaskReceiveTimerRegister {
  static constexpr uint8_t address = 0x0E;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t mrt : 8;
};

struct [[gnu::packed]] NoResponseTimerRegister1 {
  static constexpr uint8_t address = 0x0F;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t nrf_msb : 8;
};

struct [[gnu::packed]] NoResponseTimerRegister2 {
  static constexpr uint8_t address = 0x10;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t nrf_lsb : 8;
};

struct [[gnu::packed]] TimerControlRegister {
  static constexpr uint8_t address = 0x11;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class TriggerSource : uint8_t {
    DirectCmdOnly = 0b000,
    EndOfRx = 0b001,
    StartOfRx = 0b010,
    EndOfTx = 0b011,
  };

  uint8_t nrt_step : 1;
  uint8_t nrt_emv : 1;
  uint8_t : 3;
  TriggerSource source : 3;
};

struct [[gnu::packed]] GeneralPurposeTimerRegister1 {
  static constexpr uint8_t address = 0x12;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t gpt_msb : 8;
};

struct [[gnu::packed]] GeneralPurposeTimerRegister2 {
  static constexpr uint8_t address = 0x13;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t gpt_lsb : 8;
};

struct [[gnu::packed]] MainInterruptConfigRegister {
  static constexpr uint8_t address = 0x14;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t : 2;
  bool mask_col : 1;
  bool mask_txe : 1;
  bool mask_rxe : 1;
  bool mask_rxs : 1;
  bool mask_wl : 1;
  bool mask_osc : 1;
};

struct [[gnu::packed]] MaskTimerAndNfcInterruptRegister {
  static constexpr uint8_t address = 0x15;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool mask_nfct : 1;
  bool mask_cat : 1;
  bool mask_cac : 1;
  bool mask_eof : 1;
  bool mask_eon : 1;
  bool mask_gpe : 1;
  bool mask_nre : 1;
  bool mask_dct : 1;
};

struct [[gnu::packed]] MaskErrorAndWakeUpInterruptRegister {
  static constexpr uint8_t address = 0x16;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool mask_wcap : 1;
  bool mask_wph : 1;
  bool mask_wam : 1;
  bool mask_wt : 1;
  bool mask_err1 : 1;
  bool mask_err2 : 1;
  bool mask_par : 1;
  bool mask_crc : 1;
};

struct [[gnu::packed]] MainInterruptRegister {
  static constexpr uint8_t address = 0x17;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool err : 1;
  bool tim_or_nfc : 1;
  bool col : 1;
  bool txe : 1;
  bool rxe : 1;
  bool rxs : 1;
  bool wl : 1;
  bool osc : 1;
};

// Can be &-ed with MaskTimerAndNfcInterruptRegister
// (has the same structure).
struct [[gnu::packed]] TimerAndNfcInterruptRegister {
  static constexpr uint8_t address = 0x18;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool nfct : 1;
  bool cat : 1;
  bool cac : 1;
  bool eof : 1;
  bool eon : 1;
  bool gpe : 1;
  bool nre : 1;
  bool dct : 1;
};

// Can be &-ed with MaskErrorAndWakeUpInterruptRegister
// (has the same structure).
struct [[gnu::packed]] ErrorAndWakeUpInterruptRegister {
  static constexpr uint8_t address = 0x19;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool wcap : 1;
  bool wph : 1;
  bool wam : 1;
  bool wt : 1;
  bool err1 : 1;
  bool err2 : 1;
  bool par : 1;
  bool crc : 1;
};

struct [[gnu::packed]] FifoStatusRegister1 {
  static constexpr uint8_t address = 0x1A;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t fifo_bytes: 7;
  uint8_t : 1;
};

struct [[gnu::packed]] FifoStatusRegister2 {
  static constexpr uint8_t address = 0x1B;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool np_lb: 1;
  uint8_t fifo_bits: 3;
  bool overflow: 1;
  bool underflow: 1;
  uint8_t : 1;
};

struct [[gnu::packed]] CollisionDisplayRegister {
  static constexpr uint8_t address = 0x1C;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool c_pb : 1;
  uint8_t c_bits : 3;
  uint8_t c_bytes : 4;
};

struct [[gnu::packed]] NumberOfTransmittedBytesRegister1 {
  static constexpr uint8_t address = 0x1D;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t ntx_msb : 8;
};

struct [[gnu::packed]] NumberOfTransmittedBytesRegister2 {
  static constexpr uint8_t address = 0x1E;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t nbtx : 3;
  uint8_t ntx_lsb : 5;
};

struct [[gnu::packed]] NfcipBitRateDetectionDisplayRegister {
  static constexpr uint8_t address = 0x1F;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t : 4;
  BitRate nfc_rate : 4;
};

struct [[gnu::packed]] ADConverterOutputRegister {
  static constexpr uint8_t address = 0x20;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t ad : 8;
};

struct [[gnu::packed]] AntennaCalibrationControlRegister {
  static constexpr uint8_t address = 0x21;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t : 3;
  bool tre_0 : 1;
  bool tre_1 : 1;
  bool tre_2 : 1;
  bool tre_3 : 1;
  bool trim_s : 1;
};

struct [[gnu::packed]] AntennaCalibrationTargetRegister {
  static constexpr uint8_t address = 0x22;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t act : 8;
};

struct [[gnu::packed]] AntennaCalibrationDisplayRegister {
  static constexpr uint8_t address = 0x23;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t : 3;
  bool tri_err : 1;
  bool tri_0 : 1;
  bool tri_1 : 1;
  bool tri_2 : 1;
  bool tri_3 : 1;
};

struct [[gnu::packed]] AmModulationDepthControlRegister {
  static constexpr uint8_t address = 0x24;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t : 1;
  uint8_t mod : 6;
  bool am_s : 1;
};

struct [[gnu::packed]] AmModulationDepthDisplayRegister {
  static constexpr uint8_t address = 0x25;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t md: 8;
};

struct [[gnu::packed]] RfoAmModulatedLevelDefinitionRegister {
  static constexpr uint8_t address = 0x26;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool dram0 : 1;
  bool dram1 : 1;
  bool dram2 : 1;
  bool dram3 : 1;
  bool dram4 : 1;
  bool dram5 : 1;
  bool dram6 : 1;
  bool dram7 : 1;
};

struct [[gnu::packed]] RfoNormalLevelDefinitionRegister {
  static constexpr uint8_t address = 0x27;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool droff0 : 1;
  bool droff1 : 1;
  bool droff2 : 1;
  bool droff3 : 1;
  bool droff4 : 1;
  bool droff5 : 1;
  bool droff6 : 1;
  bool droff7 : 1;
};

// No register at 0x28

struct [[gnu::packed]] ExternalFieldDetectorThresholdRegister {
  static constexpr uint8_t address = 0x29;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class PeerDetectionThreshold : uint8_t {
    mv075 = 0b000,
    mv105 = 0b001,
    mv150 = 0b010,
    mv205 = 0b011,
    mv290 = 0b100,
    mv400 = 0b101,
    mv560 = 0b110,
    mv800 = 0b111,
  };

  enum class CollisionAvoidanceThreshold : uint8_t {
    mv075 = 0b0000,
    mv105 = 0b0001,
    mv150 = 0b0010,
    mv205 = 0b0011,
    mv290 = 0b0100,
    mv400 = 0b0101,
    mv560 = 0b0110,
    mv800 = 0b0111,

    mv025 = 0b1000,
    mv033 = 0b1001,
    mv047 = 0b1010,
    mv064 = 0b1011,
    mv090 = 0b1100,
    mv125 = 0b1101,
    mv175 = 0b1110,
    mv250 = 0b1111,
  };

  CollisionAvoidanceThreshold rfe : 4;
  PeerDetectionThreshold trg : 3;
  uint8_t : 1;
};

struct [[gnu::packed]] RegulatorVoltageControlRegister {
  static constexpr uint8_t address = 0x2A;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class MeasurementSource : uint8_t {
    VDD = 0b00,
    VSP_A = 0b01,
    VSP_D = 0b10,
    VSP_RF = 0b11,
  };

  uint8_t : 1;
  MeasurementSource mpsv : 2;
  uint8_t rege: 4;
  bool reg_s: 1;
};

struct [[gnu::packed]] RegulatorAndTimerDisplayRegister {
  static constexpr uint8_t address = 0x2B;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool mrt_on : 1;
  bool nrt_on : 1;
  bool gpt_on : 1;
  uint8_t : 1;
  uint8_t reg : 4;
};

struct [[gnu::packed]] RssiDisplayRegister {
  static constexpr uint8_t address = 0x2C;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t rssi_pm : 4;
  uint8_t rssi_am : 4;
};

struct [[gnu::packed]] GainReductionStateRegister {
  static constexpr uint8_t address = 0x2D;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t gs_pm : 4;
  uint8_t gs_am : 4;
};

struct [[gnu::packed]] CapacitiveSensorControlRegister {
  static constexpr uint8_t address = 0x2E;
  static constexpr RegisterKind kind = RegisterKind::RW;

  enum class Gain : uint8_t {
    VpF_2_8 = 0b000,
    VpF_6_5 = 0b001,
    VpF_1_1 = 0b010,
    VpF_0_5 = 0b100,
    VpF_0_35 = 0b110,
  };

  Gain cs_g : 3;
  uint8_t cs_mcal: 5;
};

struct [[gnu::packed]] CapacitiveSensorDisplayRegister {
  static constexpr uint8_t address = 0x2F;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t : 1;
  bool cs_cal_err : 1;
  bool cs_cal_end : 1;
  uint8_t cs_cal : 5;
};

struct [[gnu::packed]] AuxiliaryDisplayRegister {
  static constexpr uint8_t address = 0x30;
  static constexpr RegisterKind kind = RegisterKind::R;

  bool en_ac : 1;
  bool nfc_t : 1;
  bool rx_act : 1;
  bool rx_on : 1;
  bool osc_ok : 1;
  bool tx_on : 1;
  bool efd_o : 1;
  uint8_t a_cha : 1;
};

struct [[gnu::packed]] WakeUpTimerControlRegister {
  static constexpr uint8_t address = 0x31;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool wph : 1;
  bool wam : 1;
  bool wto : 1;
  // Typical wake-up time is (wut + 1) * (wur ? 10 : 100) ms.
  uint8_t wut : 3;
  bool wur : 1;
};

struct [[gnu::packed]] AmplitudeMeasurementConfigurationRegister {
  static constexpr uint8_t address = 0x32;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool am_ae : 1;
  uint8_t am_aew: 2;
  bool am_aam : 1;
  uint8_t am_d: 4;
};

struct [[gnu::packed]] AmplitudeMeasurementReferenceRegister {
  static constexpr uint8_t address = 0x33;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t am_ref : 8;
};

struct [[gnu::packed]] AmplitudeMeasurementAutoAveragingDisplayRegister {
  static constexpr uint8_t address = 0x34;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t amd_aad : 8;
};

struct [[gnu::packed]] AmplitudeMeasurementDisplayRegister {
  static constexpr uint8_t address = 0x35;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t am_amd : 8;
};

struct [[gnu::packed]] PhaseMeasurementConfigurationRegister {
  static constexpr uint8_t address = 0x36;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool pm_ae : 1;
  uint8_t pm_aew: 2;
  bool pm_aam : 1;
  uint8_t pm_d: 4;
};

struct [[gnu::packed]] PhaseMeasurementReferenceRegister {
  static constexpr uint8_t address = 0x37;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t pm_ref : 8;
};

struct [[gnu::packed]] PhaseMeasurementAutoAveragingDisplayRegister {
  static constexpr uint8_t address = 0x38;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t pm_aad : 8;
};

struct [[gnu::packed]] PhaseMeasurementDisplayRegister {
  static constexpr uint8_t address = 0x39;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t pm_amd : 8;
};

struct [[gnu::packed]] CapacitanceMeasurementConfigurationRegister {
  static constexpr uint8_t address = 0x3A;
  static constexpr RegisterKind kind = RegisterKind::RW;

  bool cm_ae : 1;
  uint8_t cm_aew: 2;
  bool cm_aam : 1;
  uint8_t cm_d: 4;
};

struct [[gnu::packed]] CapacitanceMeasurementReferenceRegister {
  static constexpr uint8_t address = 0x3B;
  static constexpr RegisterKind kind = RegisterKind::RW;

  uint8_t cm_ref : 8;
};

struct [[gnu::packed]] CapacitanceMeasurementAutoAveragingDisplayRegister {
  static constexpr uint8_t address = 0x3C;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t cm_aad : 8;
};

struct [[gnu::packed]] CapacitanceMeasurementDisplayRegister {
  static constexpr uint8_t address = 0x3D;
  static constexpr RegisterKind kind = RegisterKind::R;

  uint8_t cm_amd : 8;
};

struct [[gnu::packed]] IcIdentityRegister {
  static constexpr uint8_t address = 0x3F;
  static constexpr RegisterKind kind = RegisterKind::R;

  enum class Revision : uint8_t {
    r3_1 = 0b010,
    r3_3 = 0b011,
    r4_0 = 0b100,
    r4_1 = 0b101,
  };

  uint8_t ic_rev: 3;
  uint8_t ic_type: 5;
};

enum class DirectCommands : uint8_t {
  // Puts ST25R3911B in default state (same as after power-up)
  SetDefault = 0xC1,

  // Stops all activities and clears FIFO
  Clear = 0xC2,

  // Starts a transmit sequence using automatic CRC generation
  TransmitWithCrc = 0xC4,

  // Starts a transmit sequence without automatic CRC generation
  TransmitWithoutCrc = 0xC5,

  // Transmits REQA command (ISO14443A mode only)
  TransmitReqA = 0xC6,

  // Transmits WUPA command (ISO14443A mode only)
  TransmitWupA = 0xC7,

  // Performs initial RF collision avoidance and switch on the field
  // After termination of this command cat or cac IRQ is sent.
  NfcInitialFieldOn = 0xC8,

  // Performs response RF collision avoidance and switch on the field
  // After termination of this command cat or cac IRQ is sent.
  NfcResponseFieldOn = 0xC9,

  // Performs response RF collision avoidance with n=0 and switch on the field
  // After termination of this command cat or cac IRQ is sent.
  NfcResponseFieldOnN0 = 0xCA,

  // Accepted in NFCIP-1 active communication bit rate detection mode
  GoToNormalNfcMode = 0xCB,

  // Presets Rx and Tx configuration based on state of Mode definition register and Bit rate definition register
  AnalogPreset = 0xCC,

  // Receive after this command is ignored
  MaskReceiveData = 0xD0,

  // Receive data following this command is normally processed (this command has priority over internal Mask Receive timer)
  UnmaskReceiveData = 0xD1,

  // Amplitude of signal present on RFI inputs is measured, result is stored in A/D converter output register
  // Interrupts after termination.
  MeasureAmplitude = 0xD3,

  // Performs gain reduction based on the current noise level
  Squelch = 0xD4,

  // Clears the current squelch setting and loads the manual gain reduction from Receiver configuration register 1
  ResetRxGain = 0xD5,

  // Adjusts supply regulators according to the current supply voltage level
  // Interrupts after termination.
  AdjustRegulators = 0xD6,

  // Starts sequence that activates the Tx, measures the modulation depth and adapts it to comply with the specified modulation depth
  // Interrupts after termination.
  CalibrateModulationDepth = 0xD7,

  // Starts the sequence to adjust parallel capacitances connected to TRIMx_y pins so that the antenna LC tank is in resonance
  // Interrupts after termination.
  CalibrateAntenna = 0xD8,

  // Measurement of phase difference between the signal on RFO and RFI
  // Interrupts after termination.
  MeasurePhase = 0xD9,

  // Clears RSSI bits and restarts the measurement
  ClearRssi = 0xDA,

  // Amplitude of signal present on RFI inputs is measured, result is stored in A/D converter output register
  // Interrupts after termination.
  TransparentMode = 0xDC,

  // Calibrates capacitive sensor
  // Interrupts after termination.
  CalibrateCapacitiveSensor = 0xDD,

  // Perform capacitor sensor measurement
  // Interrupts after termination.
  MeasureCapacitance = 0xDE,

  // Perform power supply measurement
  // Interrupts after termination.
  MeasurePowerSupply = 0xDF,
  StartGeneralPurposeTimer = 0xE0,
  StartWakeUpTimer = 0xE1,
  StartMaskReceiveTimer = 0xE2,
  StartNoResponseTimer = 0xE3,
};

} // namespace st25r3911b

