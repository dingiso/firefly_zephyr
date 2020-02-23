#pragma once
#include <device.h>
#include <drivers/adc.h>

// Helper for measuring battery voltage.
// nRF-specific (uses nRF-specific SAADC driver).
class Battery {
public:
  Battery();

  // Returns current estimated voltage in millivolts.
  int32_t GetVoltage();
private:
  static adc_channel_cfg ChannelConfig();

  device* adc_device_;
  const adc_channel_cfg channel_config_;
  adc_sequence adc_seq_;
  int32_t buffer_[10];
};
