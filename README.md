# STM32F407 firmware: build, flash, and debug

This project builds firmware for an STM32F407VG (Cortex-M4). The physical
debug/flash profile uses OpenOCD with an ST-Link over SWD. The Renode profile
is a software simulation and does not program or connect to a physical board.

## Required software

- VS Code with the **Cortex-Debug** extension (`marus25.cortex-debug`)
- Arm GNU Toolchain for `arm-none-eabi-g++` and `arm-none-eabi-gdb`
- OpenOCD with its ST-Link and STM32F4 scripts, available as `openocd`
- Renode, available as `renode`, for the simulation profile

Make sure the toolchain, OpenOCD, and Renode executables are on the `PATH`
visible to VS Code.

## Flash and debug a physical board

The board must have an ST-Link debug interface (onboard or external), or another
compatible probe with a matching OpenOCD configuration. Install the probe's
Windows USB driver and verify that Windows recognizes its debug interface.
Connect the probe's SWD signals and ground to the target, and power the board.
An ST-Link USB connection alone does not mean the target is detected; OpenOCD
must be able to open the debug interface.

1. Open **Run and Debug** in VS Code.
2. Select **Physical STM32F407 (flash & debug via ST-Link)**.
3. Press **F5**. VS Code runs the **Flash STM32F407 via ST-Link** task first,
   which builds `main.elf` and asks OpenOCD to program, verify, and reset the
   target. It then starts the OpenOCD debug session and debugging at `main`.

To flash without starting a debug session, run **Tasks: Run Task** from the
Command Palette and select **Flash STM32F407 via ST-Link**. This task builds the
ELF first, then asks OpenOCD to program, verify, and reset the target.

These settings target an STM32F407VG using an ST-Link and SWD. Confirm the
target MCU and probe match before flashing. The example LED code also assumes
the STM32F4 Discovery LED wiring on PD12-PD15; other boards may need different
GPIO pins.

## Run the Renode simulation

Select **Renode Simulation (not physical hardware)** in **Run and Debug** and
press **F5**. The pre-launch task builds the firmware and starts Renode's GDB
server. This tests the firmware in a simulated STM32F4 Discovery platform; it
does not flash the physical board. The Windows launcher gives Renode an
isolated temporary directory for each run, so unrelated stale Renode files in
the system temporary directory cannot interfere with startup.
