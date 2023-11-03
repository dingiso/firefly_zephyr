# Set up

## First time set up

### Overview

Setup is somewhat convoluted as we want to force multiple things to work
together nicely:
* Nordic's version of Zephyr RTOS. We can use mainstream Zephyr as well,
  but it lacks some Nordic-specific stuff (e.g. DFU-related things).
* nRF Connect for VS Code. Nice to have it working for the application and board
  management in VS Code, properly working flashing & debugging, etc.
* [Pigweed](https://pigweed.dev/). We use many of the libraries, some of them are
  pretty unique (e.g. RPC-via-serial support). For better or worse, Pigweed comes
  with it's own toolchain and python virtual environment. If we want Pigweed tooling
  and python scripts to work - we **must** use this virtual environment, in
  particular we need to force nRF Connect for VS Code to also use it.

### Steps

* Install [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop).
* Launch it and install Toolchain Manager there.
* Install nRF Connect SDK 2.5.0 in Toolchain Manager.
  It will install Zephyr RTOS with some additional Nordic modules (we are going to
  use this part) and toolchains (which are we aren't going to use).
* Install nRF Connect for VS Code (Toolchain Manager will recommend that). It can
  also be done later from the VS Code itself - just install recommended extensions
  defined in `.vscode/extensions.json` - VS Code will propose to do that.
* Clone the Pigweed repo so `pigweed` folder is in the same folder as
  `firefly_zephyr`. Command: `git clone https://github.com/google/pigweed.git`.
* Do the next steps in the same console, as further step rely on environment
  variables set by earlier steps.
* Bootstrap Pigweed environment (`source pigweed/bootstrap.sh` or
  `pigweed/bootstrap.bat` depending on OS). This will install Pigweed toolchain
  (including python), initialize and populate python virtual environment, set up
  some environment variables.
* Install `west` (to the Pigweed's python's venv): `pip install west`.
* Install Zephyr RTOS python dependencies (again, to the Pigweed's python's venv!):
  `pip install -r ncs/v2.5.0/zephyr/scripts/requirements.txt`. `ncs` is the root
  folder for the Nordic SDKs, change the path accordingly.
* **Hopefully temporary**.  Downgrade `protobuf` python package:
  `pip install protobuf==3.20.3`. Zephyr's `requirements.txt` doesn't specify
  versions explicitly, so the command above will install latest `protobuf`.
  But it also requires `protoc` (proto compiler binary) of the version newer than
  Pigweed toolchain contains. Need to watch [this line](https://github.com/google/pigweed/blob/ac01b0bc23319c3ed8b29d9b306c808caba68616/pw_env_setup/py/pw_env_setup/cipd_setup/pigweed.json#L50),
  when it will be bumped to at least `3.20`, this step probably won't be needed
  anymore.
* Start VS Code (from the same console!): `code .`
* Add the following to your VS Code `settings.json`
  (Global one, not the one in repo):
  ```json
  "nrf-connect.west.env": {
    "GNUARMEMB_TOOLCHAIN_PATH": "????/pigweed/environment/cipd/packages/arm"
  }
  ```
  (replace ???? with the path to your pigweed folder).
* Try to build and flash something!
* When running commands from console - make sure to use `nRF Connect` one. It should
  be default one because of the `settings.json`. It's important to use it and not
  some other one as it has environment variables pointing to Zephyr installation
  we use (Nordic's one).

## Subsequent set up

* Do the next stays in the same console, as further step rely on environment
  variables set by earlier steps.
* Activate Pigweed environment (`source pigweed/activate.sh` or
  `pigweed/activate.bat` depending on OS).
* Start VS Code (from the same console!): `code .`

# Flashing

## Black Magic Probe

Doesn't use OpenOCD, has an internal GDB server to connect to directly.

To use via `west flash`,

* `board.cmake` needs to have `include(${ZEPHYR_BASE}/boards/common/blackmagicprobe.board.cmake)`
* Run `west flash --runner blackmagicprobe --gdb-serial \.\\COM11` (note weird format of COM-port name when it has 2 digits)