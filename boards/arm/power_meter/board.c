#include <zephyr/init.h>
#include <hal/nrf_power.h>

static int board_power_meter_init()
{
  // By defaul, VDD/GPIO output voltage is set to 1.8 volts and that is not enough to turn the
  // LED on. Neither it is enough to power INA226 chip (which requires 2.7V minimum).
  // Increase VDD/GPIO voltage to 3.0 volts to fix that.
  //
  // Important note: default flash state is UICR_REGOUT0_VOUT_DEFAULT (which is 0b111).
  // We can only change 1s to 0s from the firmware (not the vice versa). So if somehow
  // REGOUT0 was set to something else already (e.g. by another firmware), full chip erase
  // is needed to reset it to 0b111 (and then this code will set it to the correct value).
  if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) == (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos)) {
    // Enable write access to the UICR registers.
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
    };

    NRF_UICR->REGOUT0 =
        (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) | (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

    // Disable write access to the UICR registers.
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
    };

    // A reset is required for changes to take effect.
    NVIC_SystemReset();
	}

	return 0;
}

SYS_INIT(board_power_meter_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);