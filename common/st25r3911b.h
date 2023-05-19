#pragma once

#include "generic_device.h"

namespace st25r3911b {

struct Address {
  uint8_t address : 6;
  bool read : 1;
  uint8_t unused : 1;
};

struct IoConfigurationRegister1 {
  static constexpr uint8_t address = 0x00;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct IoConfigurationRegister2 {
  static constexpr uint8_t address = 0x01;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct OperationControlRegister {
  static constexpr uint8_t address = 0x02;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct ModeDefinitionRegister {
  static constexpr uint8_t address = 0x03;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct BitRateDefinitionRegister {
  static constexpr uint8_t address = 0x04;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct Iso14443ASettingsRegister {
  static constexpr uint8_t address = 0x05;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct Iso14443BSettingsRegister1 {
  static constexpr uint8_t address = 0x06;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct Iso14443BSettingsRegister2 {
  static constexpr uint8_t address = 0x07;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct StreamModeDefinitionRegister {
  static constexpr uint8_t address = 0x08;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AuxiliaryDefinitionRegister {
  static constexpr uint8_t address = 0x09;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct ReceiverConfigurationRegister1 {
  static constexpr uint8_t address = 0x0A;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct ReceiverConfigurationRegister2 {
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

struct ReceiverConfigurationRegister3 {
  static constexpr uint8_t address = 0x0C;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct ReceiverConfigurationRegister4 {
  static constexpr uint8_t address = 0x0D;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct MaskReceiveTimerRegister {
  static constexpr uint8_t address = 0x0E;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct NoResponseTimerRegister1 {
  static constexpr uint8_t address = 0x0F;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct NoResponseTimerRegister2 {
  static constexpr uint8_t address = 0x10;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct TimerControlRegister {
  static constexpr uint8_t address = 0x11;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct GeneralPurposeTimerRegister1 {
  static constexpr uint8_t address = 0x12;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct GeneralPurposeTimerRegister2 {
  static constexpr uint8_t address = 0x13;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct MainInterruptConfigRegister {
  static constexpr uint8_t address = 0x14;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct MaskTimerAndNfcInterruptConfigRegister {
  static constexpr uint8_t address = 0x15;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct MaskErrorAndWakeUpInterruptRegister {
  static constexpr uint8_t address = 0x16;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct MainInterruptRegister {
  static constexpr uint8_t address = 0x17;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct MaskTimerAndNfcInterruptRegister {
  static constexpr uint8_t address = 0x18;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct ErrorAndWakeUpInterruptRegister {
  static constexpr uint8_t address = 0x19;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct FifoStatusRegister1 {
  static constexpr uint8_t address = 0x1A;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct FifoStatusRegister2 {
  static constexpr uint8_t address = 0x1B;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct CollisionDisplayRegister {
  static constexpr uint8_t address = 0x1C;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct NumberOfTransmittedBytesRegister1 {
  static constexpr uint8_t address = 0x1D;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct NumberOfTransmittedBytesRegister2 {
  static constexpr uint8_t address = 0x1E;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct NfcipBitRateDetectionDisplayRegister {
  static constexpr uint8_t address = 0x1F;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct ADConverterOutputRegister {
  static constexpr uint8_t address = 0x20;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct AntennaCalibrationControlRegister {
  static constexpr uint8_t address = 0x21;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AntennaCalibrationTargetRegister {
  static constexpr uint8_t address = 0x22;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AntennaCalibrationDisplayRegister {
  static constexpr uint8_t address = 0x23;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct AmModulationDepthControlRegister {
  static constexpr uint8_t address = 0x24;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AmModulationDepthDisplayRegister {
  static constexpr uint8_t address = 0x25;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct RfoAmModulatedLevelDefinitionRegister {
  static constexpr uint8_t address = 0x26;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct RfoNormalLevelDefinitionRegister {
  static constexpr uint8_t address = 0x27;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

// No register at 0x28

struct ExternalFieldDetectorThresholdRegister {
  static constexpr uint8_t address = 0x29;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct RegulatorVoltageControlRegister {
  static constexpr uint8_t address = 0x2A;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct RegulatorAndTimerDisplayRegister {
  static constexpr uint8_t address = 0x2B;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct RssiDisplayRegister {
  static constexpr uint8_t address = 0x2C;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct GainReductionStateRegister {
  static constexpr uint8_t address = 0x2D;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct CapacitiveSensorControlRegister {
  static constexpr uint8_t address = 0x2E;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct CapacitiveSensorDisplayRegister {
  static constexpr uint8_t address = 0x2F;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct AuxiliaryDisplayRegister {
  static constexpr uint8_t address = 0x30;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct WakeUpTimerControlRegister {
  static constexpr uint8_t address = 0x31;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AmplitudeMeasurementConfigurationRegister {
  static constexpr uint8_t address = 0x32;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AmplitudeMeasurementReferenceRegister {
  static constexpr uint8_t address = 0x33;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct AmplitudeMeasurementAutoAveragingDisplayRegister {
  static constexpr uint8_t address = 0x34;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct AmplitudeMeasurementDisplayRegister {
  static constexpr uint8_t address = 0x35;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct PhaseMeasurementConfigurationRegister {
  static constexpr uint8_t address = 0x36;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct PhaseMeasurementReferenceRegister {
  static constexpr uint8_t address = 0x37;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct PhaseMeasurementAutoAveragingDisplayRegister {
  static constexpr uint8_t address = 0x38;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct PhaseMeasurementDisplayRegister {
  static constexpr uint8_t address = 0x39;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct CapacitanceMeasurementConfigurationRegister {
  static constexpr uint8_t address = 0x3A;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct CapacitanceMeasurementReferenceRegister {
  static constexpr uint8_t address = 0x3B;
  static constexpr RegisterKind kind = RegisterKind::RW;
};

struct CapacitanceMeasurementAutoAveragingDisplayRegister {
  static constexpr uint8_t address = 0x3C;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct CapacitanceMeasurementDisplayRegister {
  static constexpr uint8_t address = 0x3D;
  static constexpr RegisterKind kind = RegisterKind::R;
};

struct [[gnu::packed]] IcIdentityRegister {
  static constexpr uint8_t address = 0x3F;
  static constexpr RegisterKind kind = RegisterKind::R;

  enum class Revision {
    r3_1 = 0b010,
    r3_3 = 0b011,
    r4_0 = 0b100,
    r4_1 = 0b101,
  };

  uint8_t ic_rev: 3;
  uint8_t ic_type: 5;
};

} // namespace st25r3911b