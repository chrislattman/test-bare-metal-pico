# Raspberry Pi Pico 2 and 2 W example

Instructions:

- Clone https://github.com/raspberrypi/pico-sdk
- In `~/.bashrc` add `export PICO_SDK_PATH=/path/to/pico-sdk`
- In the `pico-sdk` folder run `git submodule update --init`
- Run `sudo apt install build-essential cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib`
- Clone https://github.com/raspberrypi/picotool
- Run `sudo apt install pkg-config libusb-1.0-0-dev`
- In `~/.bashrc` add `export PKG_CONFIG_PATH="/usr/lib/x86_64-linux-gnu/pkgconfig/"`
- In the `picotool` folder run 
    ```
    cmake -S . -B build
    cmake --build build
    sudo cmake --install build
    sudo cp udev/60-picotool.rules /etc/udev/rules.d/
    sudo udevadm control --reload
    ```

To generate Makefiles: `cmake -DPICO_BOARD=pico2_w -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-g++ -S . -B build`
- Change `pico2_w` to `pico2` if building for the Raspberry Pi Pico 2

To build application: `cmake --build build`

To run on board: 

- Unplug USB cable from board
- Hold down BOOTSEL button while plugging in USB cable
- Run `cp build/hello_world.uf2 /media/$USER/RP2350` (flashes the board with the .uf2 file)
    - Alternatively, run `picotool load -u -v -x build/hello_world.elf`

Debugging notes:

- To perform true debugging of code running on a Raspberry Pi Pico 2, you would need a [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html), a Raspberry Pi Pico 2 with header, and a breadboard, for UART serial
- Once the necessary hardware is acquired, debugging involves running `openocd` which connects to the CMSIS-DAP debug probe (alternative to J-Link) and exposes a GDB server, and the debug probe communicates to the Pico 2 using SWD (alternative to JTAG)
- For Rust, `probe-rs` replaces OpenOCD
- PlatformIO is a convenient ecosystem that bundles a lot of tools together
