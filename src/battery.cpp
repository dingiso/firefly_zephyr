#include "battery.h"

#include <hal/nrf_saadc.h>

namespace {
const uint8_t kAdcResolution = 10;
const uint8_t kAdcOversampling = 4;
static s32_t sample_buffer[10];
}

Battery::Battery():
  adc_device_(device_get_binding(DT_ADC_0_NAME)),
  channel_config_(ChannelConfig()),
  adc_seq_({
    .options      = nullptr,
    .channels     = BIT(0),
    .buffer       = sample_buffer,
    .buffer_size  = sizeof(sample_buffer),
    .resolution   = kAdcResolution,
    .oversampling = kAdcOversampling,
    .calibrate    = true
	})
{
  adc_channel_setup(adc_device_, &channel_config_);
}

int32_t Battery::GetVoltage() {
	adc_read(adc_device_, &adc_seq_);
  adc_raw_to_millivolts(adc_ref_internal(adc_device_), ADC_GAIN_1_6, kAdcResolution, sample_buffer);

  // We don't need to calibrate anymore
  adc_seq_.calibrate = false;

  return sample_buffer[0];
}

adc_channel_cfg Battery::ChannelConfig() {
  adc_channel_cfg config;
  config.gain = ADC_GAIN_1_6;
  config.reference = ADC_REF_INTERNAL;
  config.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10);
  config.channel_id = 0;
  config.input_positive = NRF_SAADC_INPUT_VDD;
  return config;
}
