## Flashing

### Black Magic Probe

Doesn't use OpenOCD, has an internal GDB server to connect to directly.

To use via `west flash`,

* `board.cmake` needs to have `include(${ZEPHYR_BASE}/boards/common/blackmagicprobe.board.cmake)`
* Run `west flash --runner blackmagicprobe --gdb-serial \.\\COM11` (note weird format of COM-port name when it has 2 digits)