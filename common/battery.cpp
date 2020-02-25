#include "battery.h"

#include <hal/nrf_saadc.h>


#include <logging/log.h>
LOG_MODULE_DECLARE();

namespace {
const uint8_t kAdcResolution = 10;
const uint8_t kAdcOversampling = 4;

adc_channel_cfg ChannelConfig() {
  adc_channel_cfg config;
  config.gain = ADC_GAIN_1_6;
  config.reference = ADC_REF_INTERNAL;
  config.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10);
  config.channel_id = 0;
  config.input_positive = NRF_SAADC_INPUT_VDD;
  return config;
}
}

Battery::Battery():
  adc_device_(device_get_binding(DT_ADC_0_NAME)),
  channel_config_(ChannelConfig()),
  adc_seq_({
    .options      = nullptr,
    .channels     = BIT(0),
    .buffer       = &buffer_,
    .buffer_size  = sizeof(buffer_),
    .resolution   = kAdcResolution,
    .oversampling = kAdcOversampling,
    .calibrate    = true
	})
{
  const auto err = adc_channel_setup(adc_device_, &channel_config_);
  if (err) {
    LOG_ERR("Error in adc_channel_setup: %d", err);
  }
}

int32_t Battery::GetVoltage() {
  auto err = adc_read(adc_device_, &adc_seq_);
  if (err) {
    LOG_ERR("Error in adc_read: %d", err);
  }

  int32_t raw = buffer_;

  err = adc_raw_to_millivolts(adc_ref_internal(adc_device_), ADC_GAIN_1_6, kAdcResolution, &raw);
  if (err) {
    LOG_ERR("Error in adc_raw_to_millivolts: %d", err);
  }

  // We don't need to calibrate anymore
  adc_seq_.calibrate = false;

  return raw;
}

Battery Battery::GetInstance() {
  static Battery singleton;
  return singleton;
}