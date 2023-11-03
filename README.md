# Firefly
![](https://github.com/aeremin/firefly_zephyr/workflows/CI/badge.svg)

# Software prerequisites
* Zephyr RTOS 3.4.0. See [Getting Started](https://docs.zephyrproject.org/latest/getting_started/index.html).
  When doing `west init`, use `west init zephyrproject --mr v3.4.0` to install this specific version.
  Make sure you can build some of Zephyr examples before proceeding.
* IDE: At the moment, Visual Studio Code with the bunch of plugins is used (VSCode will automatically ask to install them when
  opening this repository. Other IDEs can be also potentially used, more investigation is needed. Most of the commands are run
  from the command line anyway.
* Linux or running via WSL under Windows is recommended, as Black Magic Probe programming [doesn't work on Windows](https://github.com/zephyrproject-rtos/zephyr/issues/50789)
  When running under WSL, [usbipd](https://devblogs.microsoft.com/commandline/connecting-usb-devices-to-wsl/) can be used to allow interaction
  with the hardware from WSL.

# Hardware prerequisites
* For flashing, one of [nRF devkits](https://www.nordicsemi.com/Software-and-Tools/Development-Kits) is required.

# Building and flashing
To build of the images, mentioned below, use
```bash
west build <subfolder> --pristine
```
(`--pristine` can be omitted if previous build used same subfolder, it basically means "clean, non-incremental build")

To flash it, use
```bash
west flash
```

# Contents

This repo contains sources to build following firmware images:

## Firefly
This is the "main" image, located in the `firefly` folder.
Behaviours:
* Radio: will continously listen for the radio packets (see packet structure [here](common/magic_path_packet.h)).
  This is compatible with Locket firmware from [this branch](https://github.com/aeremin/Locket_fw/tree/7Colors) and
  with the Activator firmware described below. LED color will change according to packets received.
* Bluetooth: presents itself as connectable BLE device. Provides 2 BLE services:
  * Standard battery service, containing battery level (0-100) characteristic.
  * Custom service (UUID `8ec87060-8865-4eca-82e0-2ea8e45e8221`) exposing following characteristics:
    * "Beep" (UUID `8ec87062-8865-4eca-82e0-2ea8e45e8221`). Write-only characteristic, writing a byte to it will trigger short beep,
      byte written controls the volume.
    * "Blink" (UUID `8ec87063-8865-4eca-82e0-2ea8e45e8221`). Write-only characteristic, writing anything will trigger short series
      of blinks.

  Both of the services can be used by [dedicated Android app](https://install.appcenter.ms/users/a.eremin.msu/apps/ostranna-configurator/distribution_groups/public).
  App sources are [here](https://github.com/aeremin/ostranna_configurator).
* Battery measurement. Will continously monitor battery voltage and
  * Report it via bluetooth - see above.
  * Will disable radio and LED (will keep Bluetooth enabled) if battery level is low.

## Activator
This image can be used to trigger Firefly one or to provide some settings for it.
Behaviours:
* Radio: will continously transmit radio packets (see packet structure [here](common/magic_path_packet.h)).
* LED: will have a color corresponding to foreground color of packet being transmitted.
* Bluetooth: presents itself as connectable BLE device. Provides 2 BLE services:
  * Standard battery service, containing battery level (0-100) characteristic.
  * Custom service (UUID `8ec87060-8865-4eca-82e0-2ea8e45e8221`) exposing readable and writable characteristics corresponding to the
    fields of radio packet being transmitted. This allows to control the packet from e.g. phone.

  Both of the services can be used by [dedicated Android app](https://install.appcenter.ms/users/a.eremin.msu/apps/ostranna-configurator/distribution_groups/public).
  App sources are [here](https://github.com/aeremin/ostranna_configurator).


## Smoke test
This is a test image. Currently it only contains very basic test covering SPI usage and smooth LED transitions.
Will output the test log to the UART/RTT.

