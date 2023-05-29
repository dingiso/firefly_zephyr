#include "st25r3911b.h"

#include <pw_log/log.h>

namespace st25r3911b {

void St25r3911b::irq_pin_cb(const device* gpio, gpio_callback* cb, uint32_t pins) {
  k_sem_give(&(CONTAINER_OF(cb, St25r3911b, gpio_cb_)->irq_sem_));
}

void St25r3911b::Init() {
  InitIrq();

  auto ver = ReadRegister<IcIdentityRegister>();
  PW_LOG_DEBUG("ST25R3911B chip version: %u %u", ver.ic_rev, ver.ic_type);

  SendCommand(DirectCommand::SetDefault);

  DisableInterrupts();
  // Per datasheet, reading interrupts clears them. Do that to make sure
  // that everything is unset.
  GetInterrupts();

  EnableOscillator();

  WriteRegister(ModeDefinitionRegister{
    .nfc_ar = false,
    .om = ModeDefinitionRegister::OperationMode::Iso14443A,
    .targ = false,
  });

  WriteRegister(BitRateDefinitionRegister{
    .rx_rate = BitRate::Bpsk_106,
    .tx_rate = BitRate::Bpsk_106,
  });

  SendCommand(DirectCommand::AnalogPreset);

  uint16_t mv = MeasureVoltage(RegulatorVoltageControlRegister::MeasurementSource::VDD);
  PW_LOG_DEBUG("ST25R3911B Vdd: %d", mv);
  ModifyRegister<IoConfigurationRegister2>([mv](auto& p) {
    p.sup = mv > 3600 ? IoConfigurationRegister2::PowerSupply::v5 : IoConfigurationRegister2::PowerSupply::v3_3;
  });

  ExecuteCommand(DirectCommand::AdjustRegulators);
  ExecuteCommand(DirectCommand::CalibrateAntenna);

  NfcFieldOn();
}

void St25r3911b::InitIrq() {
  k_sem_init(&irq_sem_, 0, 1);

	PW_ASSERT(device_is_ready(irq_pin_spec_->port));
	PW_ASSERT(gpio_pin_configure_dt(irq_pin_spec_, GPIO_INPUT) == 0);
	gpio_init_callback(&gpio_cb_, St25r3911b::irq_pin_cb, BIT(irq_pin_spec_->pin));
	PW_ASSERT(gpio_add_callback(irq_pin_spec_->port, &gpio_cb_) == 0);
	PW_ASSERT(gpio_pin_interrupt_configure_dt(irq_pin_spec_, GPIO_INT_EDGE_TO_ACTIVE) == 0);
}

void St25r3911b::DisableInterrupts() {
  WriteRegister(MainInterruptConfigRegister({
    .mask_col = true,
    .mask_txe = true,
    .mask_rxe = true,
    .mask_rxs = true,
    .mask_wl = true,
    .mask_osc = true,
  }));

  WriteRegister(MaskTimerAndNfcInterruptRegister({
    .mask_nfct = true,
    .mask_cat = true,
    .mask_cac = true,
    .mask_eof = true,
    .mask_eon = true,
    .mask_gpe = true,
    .mask_nre = true,
    .mask_dct = true,
  }));

  WriteRegister(MaskErrorAndWakeUpInterruptRegister({
    .mask_wcap = true,
    .mask_wph = true,
    .mask_wam = true,
    .mask_wt = true,
    .mask_err1 = true,
    .mask_err2 = true,
    .mask_par = true,
    .mask_crc = true,
  }));
}

St25r3911b::InterruptRegisters St25r3911b::GetInterrupts() {
  // Per datasheet, the interrupt registers are reset after being read.
  return {
    ReadRegister<MainInterruptRegister>(),
    ReadRegister<TimerAndNfcInterruptRegister>(),
    ReadRegister<ErrorAndWakeUpInterruptRegister>(),
  };
}

St25r3911b::InterruptRegisters St25r3911b::WaitForInterrupt() {
  k_sem_take(&irq_sem_, K_FOREVER);
  return GetInterrupts();
}

void St25r3911b::EnableOscillator() {
  auto r = ReadRegister<OperationControlRegister>();
  if (!r.en) {
    ModifyRegister<MainInterruptConfigRegister>([](auto& p) {
      p.mask_osc = false;
    });

    r.en = true;
    WriteRegister(r);

    WaitForInterrupt();

    ModifyRegister<MainInterruptConfigRegister>([](auto& p) {
      p.mask_osc = true;
    });
  }
}

void St25r3911b::NfcFieldOn() {
  ModifyRegister<MaskTimerAndNfcInterruptRegister>([](auto& p) {
    p.mask_cat = false;
    p.mask_cac = false;
  });

  SendCommand(DirectCommand::NfcInitialFieldOn);
  auto irqs = WaitForInterrupt();

  PW_ASSERT(irqs.timer_and_nfc.cac != irqs.timer_and_nfc.cat);

  ModifyRegister<MaskTimerAndNfcInterruptRegister>([](auto& p) {
    p.mask_cat = true;
    p.mask_cac = true;
  });
}

uint16_t St25r3911b::MeasureVoltage(RegulatorVoltageControlRegister::MeasurementSource source) {
  ModifyRegister<RegulatorVoltageControlRegister>([source](auto& p) {
    p.mpsv = source;
  });
  ExecuteCommand(DirectCommand::MeasurePowerSupply);

  return (ReadRegister<ADConverterOutputRegister>().ad * 23438u) / 1000u;
}

void St25r3911b::ExecuteCommand(DirectCommand cmd) {
    ModifyRegister<MaskTimerAndNfcInterruptRegister>([](auto& p) {
      p.mask_dct = false;
    });

    SendCommand(cmd);

    WaitForInterrupt();

    ModifyRegister<MaskTimerAndNfcInterruptRegister>([](auto& p) {
      p.mask_dct = true;
    });
}

} // namespace st25r3911b