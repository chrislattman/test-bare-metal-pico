# Raspberry Pi Pico 2 W

Instructions:

- Clone https://github.com/raspberrypi/pico-sdk
- In `~/.bashrc` add `export PICO_SDK_PATH=/path/to/pico-sdk`
- In the `pico-sdk` folder run `git submodule update --init`
- Run `sudo apt install build-essential cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib`

To build Makefiles: `cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DPICO_BOARD=pico2_w -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/arm-none-eabi-g++ -S . -B build`

To build application: `cmake --build build`

To run on board: 

- Unplug USB cable from board
- Hold down BOOTSEL button while plugging in USB cable
- Run `cp build/blink_led.uf2 /media/$USER/RP2350` (flashes the board with the .uf2 file)
