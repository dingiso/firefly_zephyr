* It would be nice to enable RPC logging. There are some problems with it, though:
  - `pw_system\log_backend.cc` requires `sync::InterruptSpinLock` which has no Zephyr RTOS
    backend at the moment.
  - `pw_system\pw_system_private\log.h` being, well, private. Of course, it should be
    possible to use `pw_system` as a whole, but it can potentially mean dealing with
    even more non-implemented backends.