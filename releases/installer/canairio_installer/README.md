# CanAirIO installer

## Requirements

- Python3
- esptool package (i.e install it with: `pip3 install esptool`)
- USB Serial drivers (maybe only for Windows)

## Upload the firmware

For example, you able to upload the CanAirIO firmware to ESP32DEVKIT board, choosing the right binary and this parameters:

### ESP32 boards

``` bash
esptool --port /dev/ttyUSB0 -b 1500000 write_flash 0x0 canairio_ESP32DEVKIT_rev932_merged.bin
```

### ESP32C3 boards

``` bash
esptool --port /dev/ttyACM0 -b 1500000 --before default_reset --after hard_reset write_flash 0x0 canairio_ESP32C3_rev932_merged.bin
```

**Notes:**

- Maybe you need changing the port parameter.
- Some old boards like WEMOS, sometimes doesn't support this 1500000bps speed, remove this parameter, it will try auto selection.

## Download release

You can **download** this installer in [releases](https://github.com/kike-canaries/canairio_firmware/releases) section or [build](#Build) it for generate this directory and binaries (see bellow)

### Build (optional)

For rebuild all binaries, and this directory please first execute in the root of the CanAirIO firmware project:

``` bash
./build all && ./build installer
```

For more options for build the installer:

``` bash
./build help
```
