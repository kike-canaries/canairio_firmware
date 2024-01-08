
[![PlatformIO](https://github.com/kike-canaries/canairio_firmware/workflows/PlatformIO/badge.svg)](https://github.com/kike-canaries/canairio_firmware/actions/) ![ViewCount](https://views.whatilearened.today/views/github/kike-canaries/canairio_firmware.svg) [![Liberapay Status](http://img.shields.io/liberapay/receives/CanAirIO.svg?logo=liberapay)](https://liberapay.com/CanAirIO) [![Telegram Group](https://img.shields.io/endpoint?color=neon&style=flat-square&url=https%3A%2F%2Ftg.sumanjay.workers.dev%2Fcanairio)](https://t.me/canairio)

# CanAirIO firmware

![CanAirIO Community](/images/canairio_collage_community.jpg)

<a href="https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/android-hpma115s0/master/assets/googleplay/gplayicon.png" align="left" style="margin: 2px" width="140" ></a>

A citizen science project that employs both mobile and fixed sensors to measure air quality (PM 2.5 or CO2) and environmental variables. This is achieved using low-cost sensors and smartphones. The project is built around an ESP32 module board integrated with the [CanAirIO Android client app](https://github.com/kike-canaries/canairio_android).

<table>
	<tr>
		<td>
			Don't forget to star ⭐ this repository
		</td>
	</tr>
</table>

## Features

- Super easy [web installer](https://canair.io/installer), via Chrome or Edge
- Mobile station (via Bluetooth LE for tag GPS variables)
- Fixed station, (using only your WiFi)
- Fast WiFi credentials provisioning via Bluetooth or via
- [CLI tool](https://canair.io/docs/cli.html) alternative for configuration and provisioning
- Based on [CanAirIO Sensors Library](https://github.com/kike-canaries/canairio_sensorlib#readme) to support more sensors in the future
- Automatic firmware OTA updates (with stable/testing channels)
- Share mobile tracks via [mobile.canair.io](https://mobile.canair.io) or [CanAirIO app](https://github.com/kike-canaries/canairio_android)
- [Home Assistant](https://www.home-assistant.io/) integration, discovery and multisensor support
- Share your fixed station quickly via [Anaire Cloud](https://portal.anaire.org/d/detail/detalle?orgId=1&var-uid=U33TTGOTDA3D46&var-name=&refresh=1m)
- PAX Counter feature (default wifi sniffer sensor to count people)
- Multiple boards and sensors supported with only one firmware

## Boards supported

The [last release](https://github.com/kike-canaries/canairio_firmware/releases) of CanAirIO Device supports the next boards:

| Firmware Name | Boards supported |   Display  |  Guide and schematics  |
| ------------- |:-------------:| :-------------:| :----------------------:|
| **TTGO_TDISPLAY**     | TTGO T-Display | eTFT | [CanAirIO Bike](https://canair.io/docs/canairio_bike.html),  [CanAirIO Plantower](https://canair.io/docs/canairio_plantower.html) |
| **TTGO_T7**     | TTGO T7, D1Mini, ** | OLED 64x48 | [CanAirIO v2.1](https://www.hackster.io/canairio/build-a-low-cost-air-quality-sensor-with-canairio-bbf647), [CanAirIO IKEA](https://canair.io/docs/canairio_ikea.html) |
| **M5STICKCPLUS** | M5StickC Plus | eTFT | [CanAirIO M5StickC Plus](https://www.youtube.com/watch?v=TdX1AZ4PzBA) |
| **M5ATOM** | M5Atom Lite | OLED I2C | [M5Atom Lite sample](https://canair.io/docs/canairio_m5stack.html#m5atom-lite)|
| **ESP32DevKit** | ESP32DevKit, NodeMCU V3, ** | OLED 128x64 | [HacksterIO](https://www.hackster.io/canairio/build-low-cost-air-quality-sensor-canairio-without-soldering-d87494) |
| **TTGO_TQ** | TTGO TQ | Builtin OLED  | [TTGO_TQ board](https://de.aliexpress.com/item/10000291636371.html) |
| **WEMOSOLED** | WemosOLED and similar boards | OLED 128x64 | [ESP32 OLED board](https://de.aliexpress.com/item/33047481007.html) |
| **HELTEC** |  ESP32 Heltec board |  OLED 128x64 | |
| **ESP32C3** | M5STAMPC3** | OLED | BLE not supported |
| **ESP32C3OIPLUS** | TTGO-T-OI-Plus | OLED | BLE not supported |
| **ESP32C3LOLIN** | LOLIN Mini C3 | OLED | BLE not supported |
| **ESP32C3SEEDX** | Seeed_xiao_esp32c3 | OLED | BLE not supported |
| **ESP32S3** | T7S3 and Makerfabs | - | In development |

** is possible that the **current firmware supports more boards** and sensors. Also you can choose the sensor brand or type on the CanAirIO Android app or on the firmware CLI.

# Installation alternatives

We have different alternatives to load the current firmware. By complexity order, they are:

## Via CanAirIO Web Installer (RECOMMENDED)

If you already have a ESP32 board, you can test our CanAirIO firmware on one click, with our web installer:

[![canairio web installer on m5stickcplus](https://user-images.githubusercontent.com/423856/152767232-81c11957-26f0-4a83-bf63-6a4bee41a168.gif)](https://youtu.be/TdX1AZ4PzBA)  
[Full video](https://youtu.be/TdX1AZ4PzBA) - [Web installer](https://canair.io/installer.html)

## Via CanAirIO loader

You will able to install the last version of CanAirIO firmware using a simple Arduino sketch that it will doing all for you, you only need to use the official [Arduino IDE](https://www.arduino.cc/en/software) or [Arduino Droid app for Android](https://play.google.com/store/apps/details?id=name.antonsmirnov.android.arduinodroid2&hl=en&gl=US) for load this [simple sketch](https://github.com/hpsaturn/esp32-canairio-loader/blob/master/canairio_loader/canairio_loader.ino). Please follow the instructions [here](https://github.com/hpsaturn/esp32-canairio-loader) or follow the next [YouTube video guide](https://youtu.be/FjfGdnTk-rc) for Android OTG installation alternative.

## Via binaries

You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section. For example, download the last **production** release from `assets` section, like this:  

![releases assets](images/assets.jpg)

please uncompress the zip file and connect your CanAirIO device to your USB and execute the next command to upload the firmware to your board, for example for an ESP32DevKit board you should run the next commands:

### Linux and MacOSx

``` bash
unzip canairio_rev414_20190829.zip
cd canairio_installer
esptool --port /dev/ttyUSB0 -b 1500000 write_flash 0x0 canairio_ESP32DEVKIT_rev932_merged.bin
```

(You should install [esptool](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html) in your system.)

### Windows

Please read the [Espressif Uploader](https://canair.io/docs/firmware_upload.html#espressif-uploader) section in the main documentation to have details of how load the firmware via the official **Espressif Download Tool** in Windows.

## Via PlatformIO (Compiling on Linux, Mac or Windows)

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

For the **default** board `TTGO_TDISPLAY` (CanAirIO Bike), clone and upload firmware via USB cable:

``` bash
git clone https://github.com/kike-canaries/canairio_firmware
cd canairio_firmware
pio run --target upload
```

For a specific firmware for example for a TTGO-T7 board, only change the last line for:

``` bash
pio run -e TTGO_T7 --target upload
```

## Via Docker

First build the Docker image using the following command line:

```bash
docker build -t canairio_pio:master .
```

This build a basic compiler image with all PlatformIO stuff. For build the project you only needs now run:

```bash
./docker_build run
```

## OTA WAN updates

CanAirIO offers two channels for remote OTA (Over-The-Air) updates for your device: the production channel and the development channel. This means you won't need to reinstall the firmware manually for any updates; it's all automatic. You only need to have Wi-Fi enabled on your device to receive these firmware updates.

If you're interested in the latest testing updates, please go to the releases section and choose and download the development firmware (a zip file with `dev` in its name), then upload it to your board to receive these kind of updates.

# Supporting the project

If you want to contribute to the code or documentation, consider posting a bug report, feature request or a pull request.

When creating a pull request, we recommend that you do the following:

- Clone the repository
- Create a new branch for your fix or feature. For example, git checkout -b fix/my-fix or git checkout -b feat/my-feature.
- Run to any clang formatter if it is a code, for example using the `vscode` formatter. We are using Google style. More info [here](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
- Document the PR description or code will be great
- Target your pull request to be merged with `devel` branch

Also you can consider make a donation, be a patron or buy a device:  


- Via **Ethereum**:
- 0x1779cD3b85b6D8Cf1A5886B2CF5C53a0E072C108
- Be a patron: [Github Sponsors](https://github.com/sponsors/hpsaturn), [LiberaPay](https://liberapay.com/CanAirIO)
- **Buy a device**: [CanAirIO Bike in Tindie](https://www.tindie.com/products/hpsaturn/canairio-bike/)
- Inviting us **a coffee**: [buymeacoffee](https://www.buymeacoffee.com/hpsaturn), [Sponsors](https://github.com/sponsors/hpsaturn?frequency=one-time)  

<a href="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/ethereum_donation_address.png" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/ethereum_donation_address.png" align="left" style="margin: 10px" width="140" ></a>  

**NOTE:**  
Supporting our Citizen Science Initiative many people be able to fight for air quality rights in many countries with this kind of problems. More info in [CanAir.IO](https://canair.io)  


# CanAirIO device HOWTO guide

We have some build guides with different alternatives, please visit our [CanAirIO documentation](https://canair.io/docs).

![CanAirIO CO2 and Mini](https://canair.io/docs/images/canairio_bike_co2_mini.jpg)

<a href="https://canair.io/docs" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/collage.jpg" height="400" ></a>

## Building Guide

<a href="https://youtu.be/V2eO1UN5u7Y" target="_blank" style="padding-left: 20px" ><img src="images/canairio_bike_make_of_youtube.jpg" width="420" ></a>

## Soldering Guide (some improvements)

<a href="https://youtu.be/Oarq0K0Sz3c" target="_blank" style="padding-left: 20px" ><img src="images/canairio_bike_soldering_on_youtube.jpg" width="420" ></a>

## Box STL files

** W A R N N I N G **

The last versions for all box versions, are in the [official repository](https://github.com/kike-canaries/canairio_firmware/tree/master/box) because it is more easy for handling the versions than Thingiverse.

# TODO

- [X] Enable/Disable APIs from App
- [X] Locatitation settings via Bluetooth
- [X] Sensors manager is now a library ([CanAirIO Sensorlib](https://github.com/kike-canaries/canairio_sensorlib#canairio-air-quality-sensors-library))
- [X] Auto detection of PM sensors (see sensorlib doc)
- [x] CO2 sensors in chart
- [X] OTA updates (LAN and WAN) (dev/prod, see releases for details) 
- [x] Multiple variables in chart (C02,PM2.5,Hum,Temp,etc)
- [x] Map of each recorded track in details
- [x] InfluxDB schema (by geohashes)
- [x] New pax counter variable (People around the device)
- [x] WiFi SSIDs scanner for have a choose list of it in the app
- [x] Pax counter disable/enable
- [x] Home Assistant integration (with zero-config)
- [x] Anaire cloud integration (Automatic time series of your station)
- [x] Fahrenheit and Kelvin units supported
- [x] Geiger sensor supported
- [ ] Sensor community alternativa for fixed stations
- [ ] Anonymous authentication
