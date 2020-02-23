#include <device.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <logging/log.h>
#include <zephyr.h>

#include <vector>

#include "cc1101.h"
#include "rgb_led.h"

LOG_MODULE_REGISTER();

struct MagicPathRadioPacket {
  uint8_t r, g, b;
} __attribute__((__packed__));

struct UsbHostPacket {
  uint16_t From;  // 2
  uint16_t To;    // 2
  uint8_t stuff[7];
} __attribute__ ((__packed__));

void main(void) {
  LOG_WRN("Hello! Application started successfully.");


  Cc1101 cc1101;
  cc1101.Init();
  cc1101.SetChannel(1);

  RgbLed led;

  Color colors[] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
  uint8_t currentColor = 0;
  auto t1 = RunEvery([&](){
    led.SetColorSmooth(colors[currentColor], 1000);
    currentColor = (currentColor + 1) % 3;
  }, 2000);

  while (true) {
    UsbHostPacket pkt_2;
    if (cc1101.Receive(100, &pkt_2)) {
      LOG_INF("Got packet from %d", pkt_2.From);
    }

    UsbHostPacket pkt_3;
    pkt_3.From = 17;
    cc1101.Transmit(pkt_3);

    k_sleep(100);
  }
}
