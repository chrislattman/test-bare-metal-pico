# Raspberry Pi Pico 2 and 2 W example

Instructions:

- Run `git submodule update --init --recursive && git config submodule.pico-sdk.ignore all`
- Run `sed -i "s/-DPICOTOOL_NO_LIBUSB=1/-DPICOTOOL_NO_LIBUSB=0/g" pico-sdk/tools/Findpicotool.cmake`
- Run `sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib gdb-multiarch`

If you want to install `picotool` separately:

- Clone https://github.com/raspberrypi/picotool
- Run `sudo apt install pkg-config libusb-1.0-0-dev`
- In `~/.bashrc` add `export PKG_CONFIG_PATH="/usr/lib/x86_64-linux-gnu/pkgconfig/"`
- In the `picotool` folder run 
    ```
    git checkout 2.3.0
    cmake -S . -B build
    cmake --build build
    sudo cmake --install build
    sudo cp udev/60-picotool.rules /etc/udev/rules.d/
    sudo udevadm control --reload
    ```

To generate Makefiles:

```
PICO_SDK_PATH=$(git rev-parse --show-toplevel)/pico-sdk
cmake -DPICO_BOARD=pico2 -DPICOTOOL_FORCE_FETCH_FROM_GIT=ON -S . -B build
```

- Change `pico2` to `pico2_w` if building for the Raspberry Pi Pico 2 W
- To use a toolchain that isn't on `PATH`, add `-DPICO_TOOLCHAIN_PATH=/path/to/toolchain`
- To target the RP2350's RISC-V (Hazard3) cores instead of the Cortex-M33 cores:
    - Install the generic bare-metal [xPack GNU RISC-V Embedded GCC](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases) (`riscv-none-elf`):
        ```
        mkdir -p ~/.local/xPacks/riscv-none-elf-gcc
        curl -L https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v15.2.0-1/xpack-riscv-none-elf-gcc-15.2.0-1-linux-x64.tar.gz \
            | tar xz -C ~/.local/xPacks/riscv-none-elf-gcc
        ```
    - Add `-DPICO_PLATFORM=rp2350-riscv -DPICO_TOOLCHAIN_PATH=$HOME/.local/xPacks/riscv-none-elf-gcc/xpack-riscv-none-elf-gcc-15.2.0-1` (or leave out `PICO_TOOLCHAIN_PATH` if the toolchain's `bin` folder is on `PATH`)
    - Use a separate build directory (e.g. `-B build-riscv`), since the compiler can't be changed once a build directory is configured
    - Note: the `qemu` targets are Arm-only and are not defined
- The build type defaults to `MinSizeRel` (`-Os -DNDEBUG`). Add `-DCMAKE_BUILD_TYPE=Debug` to build with `-Og -ggdb3` and, since `NDEBUG` is then undefined, it skips the `sleep_ms()` calls during debugging (switch back with `-DCMAKE_BUILD_TYPE=MinSizeRel`).

To build application and run on board: 

- Unplug USB cable from board
- Hold down BOOTSEL button while plugging in USB cable
- Run `cmake --build build --target run`

Other targets: `qemu` runs the firmware in qemu-system-arm with the serial port on telnet localhost 5678, and `qemu_debug` does the same but halts and waits for GDB on localhost:1234 (use it with -DCMAKE_BUILD_TYPE=Debug). See CMakeLists.txt for details.

Debugging notes:

- To perform true debugging of code running on a Raspberry Pi Pico 2, you would need a [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html), a Raspberry Pi Pico 2 with header, and a breadboard, for UART serial
    - You could use a USB logic analyzer and PulseView to sniff UART messages over the wire
    - You could use a FT232H adapter (don't need dual-channel FT2232H thanks to debug probe) to drive I2C, SPI, etc. signals from PC to Pico 2
- Once the necessary hardware is acquired, debugging involves running `openocd` which connects to the CMSIS-DAP debug probe (alternative to J-Link) and exposes a GDB server, and the debug probe communicates to the Pico 2 using SWD (alternative to JTAG)
    - From a Xilinx perspective, think of `openocd` as the analog to `xsdb` with a built-in `hw_server`
    - Like `xsdb`, software is loaded through `openocd` via TCL commands and both expose GDB ports
    - Other vendor-proprietary versions of OpenOCD include NXP's LinkServer and STM's ST-Link
- PlatformIO is a beginner-friendly ecosystem that bundles a lot of tools together
