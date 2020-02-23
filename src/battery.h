#pragma once
#include <device.h>
#include <drivers/adc.h>

// Helper for measuring battery voltage.
// nRF-specific (uses nRF-specific SAADC driver).
// There are 2 things which I still can't figure out
// 1) It produces weird result if Battery is not statically allocated. Probably it has
//    to do with underlying libraries somehow requiring buffer to be statically allocated.
//    So the only way to instantiate this is by using GetInstance() method which will
//    return statically allocated singleton.
// 2) For some reason first call to GetVoltage() will always (?) return 0 (more precisely:
//    something based on default value of buffer). So caller should probably ignore 0 results.
class Battery {
public:
  static Battery GetInstance();

  // Returns current estimated voltage in millivolts.
  // Don't trust it if it returns 0 - see above.
  int32_t GetVoltage();
private:
  Battery();

  device* adc_device_;
  const adc_channel_cfg channel_config_;
  adc_sequence adc_seq_;
  int16_t buffer_ = 0;
};
