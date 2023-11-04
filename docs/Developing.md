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
* Install nRF Connect for VS Code. It can also be done later from the VS Code itself -
  just install recommended extensions defined in `.vscode/extensions.json` - VS Code will propose to do that.
* Install `west` globally: `pip install west` (need to be run with administrator priviliges).
* Run `west init -l manifest && west update && west zephyr-export` (from the repository root).
  This will create and populate `third_party` folder with all dependencies.
* Do the next steps in the same console, as further step rely on environment
  variables set by earlier steps.
* Bootstrap Pigweed environment (`source pigweed/bootstrap.sh` or
  `third_party\pigweed\bootstrap.bat` depending on OS). This will install Pigweed toolchain
  (including python), initialize and populate python virtual environment, set up
  some environment variables.
* Install Zephyr RTOS python dependencies (again, to the Pigweed's python's venv!):
  `pip install -r third_party/zephyr/scripts/requirements.txt`.
* Start VS Code (from the same console!): `"C:\Program Files\Microsoft VS Code\Code.exe" .` (or just `code .` if you have it in PATH).
* Add the following to your VS Code `settings.json`
  (Global one, not the one in repo):
  ```json
  "nrf-connect.west.env": {
    "GNUARMEMB_TOOLCHAIN_PATH": "????/third_party/pigweed/environment/cipd/packages/arm"
  }
  ```
  (replace ???? with the path to this repository location).
* Try to build and flash something!
* When running commands from console - make sure to use `nRF Connect` one. It should
  be default one because of the `settings.json`. It's important to use it and not
  some other one as it has environment variables pointing to Zephyr installation
  we use (Nordic's one).

## Subsequent set up

* Do the next stays in the same console, as further step rely on environment
  variables set by earlier steps.
* Activate Pigweed environment (`source third_party/pigweed/activate.sh` or
  `third_party\pigweed\activate.bat` depending on OS).
* Start VS Code (from the same console!): `"C:\Program Files\Microsoft VS Code\Code.exe" .` (or just `code .` if you have it in PATH).

# Flashing

## Black Magic Probe

Doesn't use OpenOCD, has an internal GDB server to connect to directly.

To use via `west flash`,

* `board.cmake` needs to have `include(${ZEPHYR_BASE}/boards/common/blackmagicprobe.board.cmake)`
* Run `west flash --runner blackmagicprobe --gdb-serial \.\\COM11` (note weird format of COM-port name when it has 2 digits)