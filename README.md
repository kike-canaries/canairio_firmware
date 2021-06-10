

[![Build Status](https://travis-ci.com/kike-canaries/canairio_firmware.svg?branch=master)](https://travis-ci.com/kike-canaries/canairio_firmware) [![PlatformIO](https://github.com/kike-canaries/canairio_firmware/workflows/PlatformIO/badge.svg)](https://github.com/kike-canaries/canairio_firmware/actions/) ![ViewCount](https://views.whatilearened.today/views/github/kike-canaries/canairio_firmware.svg) [![Liberapay Status](http://img.shields.io/liberapay/receives/CanAirIO.svg?logo=liberapay)](https://liberapay.com/CanAirIO)  

<a href="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/collage_vertical.jpg" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/collage_vertical.jpg" align="right" width="190" ></a>

# CanAirIO firmware

<a href="https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/android-hpma115s0/master/assets/googleplay/gplayicon.png" align="left" width="128" ></a>

Citizen science project with mobile and fixed sensors for measuring air quality (PM 2.5) using low-cost sensors and smartphones. Built with a `ESP32` module board and a dust sensor, interfaced with an [CanAirIO Android client app](https://github.com/kike-canaries/canairio_android). The current firmware supports multiple boards and sensors, please review the **DIY guides** in our [wiki](https://github.com/kike-canaries/canairio_firmware/wiki)

## Boards supported

The [last release](https://github.com/kike-canaries/canairio_firmware/releases) of CanAirIO Device supports the next boards:

| Firmware Name | Boards supported |   Display  |  Guide and schematics  |
| ------------- |:-------------:| :-------------:| :----------------------:|
| **TTGO_T7**     | TTGO T7, D1Mini, ** | OLED 64x48 | [CanAirIO v2.1](https://www.hackster.io/canairio/build-a-low-cost-air-quality-sensor-with-canairio-bbf647) |
| **ESP32DevKit** | ESP32DevKit, NodeMCU V3, ** | OLED 128x64 | [HacksterIO](https://www.hackster.io/canairio/build-low-cost-air-quality-sensor-canairio-without-soldering-d87494) |
| **TTGO_TQ** | TTGO TQ | Builtin OLED  | [TTGO_TQ board](https://de.aliexpress.com/item/10000291636371.html) |
| **WEMOSOLED** | WemosOLED and similar boards | OLED 128x64 | [ESP32 OLED board](https://de.aliexpress.com/item/33047481007.html) |
| **HELTEC** |  ESP32 Heltec board |  OLED 128x64 | | 


** is possible that the **current firmware supports more boards** and sensors. Also you can choose the sensor brand or type on the CanAirIO Android app.


# Installation alternatives

We have different alternatives for load the current firmware. In order of complexity they are:

## Via CanAirIO loader (RECOMMENDED)

You will able to install the last version of CanAirIO firmware with internet updates via a simple Arduino sketch that it will doing all for you, you only need to use the official [Arduino IDE](https://www.arduino.cc/en/software) or [Arduino Droid app for Android](https://play.google.com/store/apps/details?id=name.antonsmirnov.android.arduinodroid2&hl=en&gl=US) for load this [simple sketch](https://github.com/hpsaturn/esp32-canairio-loader/blob/master/canairio_loader/canairio_loader.ino). Please follow the instructions [here](https://github.com/hpsaturn/esp32-canairio-loader) or follow the next [YouTube video guide](https://youtu.be/FjfGdnTk-rc) for Android OTG installation alternative.

## Via binaries (Linux and MacOSx)

You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section. Download the last release from `assets` section in releases and please uncompress zip file.

### USB alternative

Connect your CanAirIO device to your USB and execute the next command for your model board*, like this:

``` bash
unzip canairio_rev414_20190829.zip
cd canairio_installer
./install.sh canairio_TTGO_T7_rev605_20200925.bin
```

For more options run `./canairio_installer help`.  

**Note**: you need python2 or python3 with pyserial in your system.

### OTA alternative

After that you will able to send OTA updates to any board supported, like this:

``` bash
./install.sh ota canairio_d1mini_rev414_20190829.bin
```

Also you can specify the IP address:

```bash
./install.sh ota canairio_xxx.bin 192.168.1.10
```

## Via binaries (Windows)

Please read procedure on `firmware` section on [HacksterIO Guide](https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a#toc-firmware-y-software-3) for details for load firmware via oficial **Espressif Download Tool** in Windows


## Via PlatformIO (Compiling on Linux, Mac or Windows)

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

For **default** board `TTGO_T7`, clone and upload firmware via USB cable:

``` bash
git clone https://github.com/kike-canaries/esp32-hpma115s0.git
cd esp32-hpma115s0
pio run -e TTGO_T7 --target upload
```

After that, it able for sending updates via OTA protocol using Wifi in your LAN, is more fastest than USB and you can disconnect your board, but `you need first save Wifi credentials` via Android CanAirIO app [see below](#settings).

For **local OTA updates** you only run

``` bash
pio run -e TTGO_T7_OTA --target upload
```

Also you can specify the IP address on `platformio.ini` config file

**Optional** for other board, please select the right environment for example for `wemos` board:

``` bash
pio run -e WEMOSOLED --target upload
```

Also you can change in `platformio.ini` the next parameters:

-D CORE_DEBUG_LEVEL=0, set to 3 for get more verbose log.  
-D EMOTICONS, comment or uncomment for enable emoticons.  
upload_port, in OTA section is for set to CanAirIO device IP address.

#### Building Installer

You can build `CanAirIO Installer` zip package with all binaries of all board flavors running the next command:

``` bash
./build all && ./build installer
```

The directory output is in: `releases/installer`  
Also the binaries flavors directory: `releases/binaries/`

### Troubleshooting


If you have some issues with Bluetooth library internals, or libraries issues, please upgrade all frameworks and tools on PlatformIO:

``` bash
pio update
sudo pio upgrade
pio run -t clean
rm -rf .pio
pio lib update
pio run --target upload
```

If you want clear all ESP32 preferences and flash variables, please execute before:

```bash
./install.sh erase /dev/ttyUSB0
```

Also you can do it with esptool:

```bash
./esptool.py --port /dev/ttyUSB0 erase_flash
```

# Supporting the project

If you want to contribute to the code or documentation, consider posting a bug report, feature request or a pull request.

When creating a pull request, we recommend that you do the following:

- Clone the repository
- Create a new branch for your fix or feature. For example, git checkout -b fix/my-fix or git checkout -b feat/my-feature.
- Run to any clang formatter if it is a code, for example using the `vscode` formatter. We are using Google style. More info [here](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
- Document the PR description or code will be great
- Target your pull request to be merged with `devel` branch

Also you can make a donation, be a patreon or buy a device:  

<a href="images/ethereum_donation_address.png" target="_blank"><img src="images/ethereum_donation_address.png" align="right" width="190" margin-left="20px" ></a>

- Via **Ethereum**: `0x1779cD3b85b6D8Cf1A5886B2CF5C53a0E072C108`
- Via **Liberapay**: [CanAirIO in LiberaPay](https://liberapay.com/CanAirIO)
- **Buy a device**: [CanAirIO Bike in Tindie](https://www.tindie.com/products/hpsaturn/canairio-bike/)
- [Inviting us **a coffee**](https://www.buymeacoffee.com/hpsaturn) 

## TODO

- [X] Enable/Disable APIs from App
- [X] Locatitation settings via Bluetooth
- [X] Sensors manager is now a library ([CanAirIO Sensorlib](https://github.com/kike-canaries/canairio_sensorlib#canairio-air-quality-sensors-library))
- [X] Auto detection of PM sensors (see sensorlib doc)
- [x] CO2 sensors in chart
- [X] OTA updates (LAN and WAN) (dev/prod, see releases for details) 
- [x] Multiple variables in chart (C02,PM2.5,Hum,Temp,etc)
- [x] Map of each recorded track in details
- [ ] InfluxDB schema (by countries)
- [ ] Anonymous authentication

---

## CanAirIO device HOWTO guide

We have some build guides with different alternatives, please visit our [CanAirIO documentation](https://canair.io/docs).

<a href="https://canair.io/docs" target="_blank"><img src="https://raw.githubusercontent.com/wiki/kike-canaries/canairio_firmware/images/collage.jpg" height="600" ></a>

---