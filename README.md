[![Build Status](https://travis-ci.com/kike-canaries/esp32-hpma115s0.svg?branch=master)](https://travis-ci.com/kike-canaries/esp32-hpma115s0) 

# ESP32-HPMA115S0 (CanAirIO sensor)

<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/collage.jpg" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/esp32-hpma115s0/master/images/collage.jpg" align="right" width="340" ></a>

<a href="https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter" target="_blank"><img src="https://github.com/kike-canaries/android-hpma115s0/blob/master/assets/googleplay/gplayicon.png" align="left" width="128" ></a>

Citizen science project with mobile and fixed sensors for measuring air quality (PM 2.5) using low-cost sensors and smartphones. Built with a `ESP32` module board and `HPMA115s0 Honeywell` dust sensor, interfaced with an [CanAirIO Android client app](https://github.com/kike-canaries/android-hpma115s0).

**Full guide**: [English](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Guide-(EN)) **|** [Spanish](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Guide-(ES))<br/>

## Installation

### Linux and MacOSx

You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section. Download the last release from `assets` section in releases and please uncompress zip file, connect your device and execute the next command for your model board (D1Mini, WemosOLED, Heltec) like this:

``` bash
unzip canairio_rev414_20190829.zip
cd canairio_installer
./install.sh canairio_d1mini_rev414_20190829.bin
```

**Note**: you need python2 or python3 with pyserial in your system.  
**Tip**: if you want clear all preferences and flash variables, please execute before:

``` bash
esptool.py --port /dev/ttyUSB0 erase_flash
```

After that you will able to send OTA updates to any board supported, like this:

``` bash
./install.sh ota canairio_d1mini_rev414_20190829.bin
```

### Windows

Please read procedure on `firmware` section on [HacksterIO Guide](https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a#toc-firmware-y-software-3) for details for load firmware via oficial **Espressif Download Tool** in Windows

## [Optional] Compiling and installing

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

For **default** board `D1Mini Kit like`, clone and upload firmware via USB cable:

``` bash
git clone https://github.com/kike-canaries/esp32-hpma115s0.git
cd esp32-hpma115s0
pio run -e d1mini --target upload
```

After that, it able for sending updates via OTA protocol using Wifi in your LAN, is more fastest than USB and you can disconnect your board, but `you need first send Wifi credentials` via Android CanAirIO app (see below)

For **OTA updates** you only run

``` bash
pio run --target upload
```

**Optional** for other board, please select the right environment for example for `wemos` board:

``` bash
pio run -e wemos --target upload
```

### Building Installer

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

## CanAirIO Firmware

<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/influxdb00.jpg" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/esp32-hpma115s0/master/images/influxdb00.jpg" width="512" ></a>

<a href="https://raw.githubusercontent.com/kike-canaries/esp32-hpma115s0/master/images/app_settings_tools.png" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/esp32-hpma115s0/master/images/app_settings_tools.png" align="right" width="256" ></a>

You can use it from [CanAirIO Android app](https://github.com/kike-canaries/android-hpma115s0), you can connect to your device via Bluetooth and record mobile captures and save tracks on your sdcard. Also you can share these tracks to CanAirIO network. If you want set your device for static station, please configure Wifi and CanAirIO API or a custom InfluxDb server, please see details below. Also, in our [guide](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Guide-(EN)) you have more information of how using the Android app.

### CanAirIO API configuration

Please connect your device via Bluetooth and in the settings section configure parameters like `Sample Time Interval` and `Station Name`, these stuff is for configure our API cloud or a custom influxDb instance. You can get a username and password of our API on the next [link](http://canairiofront.herokuapp.com/register) and view captures [here](http://gblabs.co:8888/sources/1/dashboards/1).

### [Optional] Custom InfluxDb server

Also you can use any `influxdb` instance and configure it via CanAirIO Android app or via nRF connect app. Please see details in [Firmware-Protocol](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Firmware-Protocol) wiki page.

---

## TODO

- [X] Enable/Disable APIs from App (on testing)
- [X] Locatitation settings via Bluetooth (on testing)
- [X] OTA updates ready (LAN)
- [ ] OTA updates (WAN)
- [ ] Migrate `loop` to multithread RTOS implementation
- [ ] Dinamic Humidity and Temperature visualization on Display

---

## CanAirIO device HOWTO guide

With the next guides, you will be able to build a device to measure air quality by using a Honeywell HPMA115SO sensor, which measures PM 2.5 and PM 10 particles and then if you want, publish it to CanAirio cloud or a personal server using [CanAirIO App](https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter)

[CanAirIO guide [English]](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Guide-(EN))  
[CanAirIO guide [Spanish]](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Guide-(ES))

<a href="https://raw.githubusercontent.com/kike-canaries/esp32-hpma115s0/master/images/collage_v2.jpg" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/esp32-hpma115s0/master/images/collage_v2.jpg" height="324" ></a>
