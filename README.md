# Raspberry Pi Pico 2 and 2 W example

Instructions:

- Run `git submodule update --init --recursive && git config submodule.pico-sdk.ignore all`
- Run `sed -i "s/-DPICOTOOL_NO_LIBUSB=1/-DPICOTOOL_NO_LIBUSB=0/g" pico-sdk/tools/Findpicotool.cmake`
- Run `sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib`

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
PICO_SDK_PATH=$(git rev-parse --show-toplevel)/pico-sdk \
cmake -DPICO_BOARD=pico2 \
    -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-gcc \
    -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-g++ \
    -DPICOTOOL_FORCE_FETCH_FROM_GIT=ON \
    -S . -B build
```

- Change `pico2` to `pico2_w` if building for the Raspberry Pi Pico 2 W

To build application and run on board: 

- Unplug USB cable from board
- Hold down BOOTSEL button while plugging in USB cable
- Run `cmake --build build --target run`

Debugging notes:

- To perform true debugging of code running on a Raspberry Pi Pico 2, you would need a [Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html), a Raspberry Pi Pico 2 with header, and a breadboard, for UART serial
    - You could use a USB logic analyzer and PulseView to sniff UART messages over the wire
    - You could use a FT232H adapter (don't need dual-channel FT2232H thanks to debug probe) to drive I2C, SPI, etc. signals from PC to Pico 2
- Once the necessary hardware is acquired, debugging involves running `openocd` which connects to the CMSIS-DAP debug probe (alternative to J-Link) and exposes a GDB server, and the debug probe communicates to the Pico 2 using SWD (alternative to JTAG)
- For Rust, `probe-rs` replaces OpenOCD
- PlatformIO is a beginner-friendly ecosystem that bundles a lot of tools together
