# ESP32-HPMA115S0 (CanAirIO sensor)

<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/collage.jpg" target="_blank"><img src="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/collage.jpg" align="right" width="384" ></a>

<a href="https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter" target="_blank"><img src="https://github.com/kike-canaries/android-hpma115s0/blob/master/assets/googleplay/gplayicon.png" align="left" width="128" ></a>

Citizen science project with mobile and fixed sensors for measuring air quality (PM 2.5) using low-cost sensors and smartphones. Built with a `ESP32` module board and `HPMA115s0 Honeywell` dust sensor, interfaced with an [CanAirIO Android client app](https://github.com/kike-canaries/android-hpma115s0).

**Full guide (Hackster.io):** [English](https://www.hackster.io/MetaKernel/canairio-citizen-network-for-air-quality-monitoring-bbf647) **|** [Spanish](https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a)


## Installation

### Linux and MacOSx

You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section. Download the last release from `assets` section in releases and please uncompress zip file, connect your device and execute the next command for your model board (D1Mini, WemosOLED, Heltec) like this:

``` bash
unzip canairio_installer_20190503rev312.zip
cd canairio_installer
./install.sh canairio_d1mini_20190503rev312.bin
```
**Note**: you need python2 or python3 with pyserial in your system.

if you want clear all preferences and flash variables, please execute before:

``` bash
./install.sh canairio_d1mini_20190503rev312.bin
```

### Windows

Please read procedure on our [HacksterIO Guide]("https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a#toc-firmware-y-software-3") for details for load firmware via oficial **Espressif Download Tool**


## [Optional] Compiling and installing

### Software Dependencies

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

For **default** board `D1Mini Kit`, clone and upload firmware via USB cable:

``` bash
git clone https://github.com/kike-canaries/esp32-hpma115s0.git
cd esp32-hpma115s0
pio run --target upload
```

**Optional** for other board, please edit and select it on `platformio.ini` file and upload the new firmware, for example for `Heltec`:

``` python
build_flags =
# Uncomment your board
# -D WEMOSOLED=1
# -D D1MINI=1
 -D HELTEC=1
```

### Troubleshooting

If you have some issues with Bluetooth library internals, please upgrade all frameworks and tools on PlatformIO:

```
pio update
sudo pio upgrade
pio run -t clean
rm -rf .pioenvs .piolibdeps
pio run --target upload
```

## Usage

From [CanAirIO Android app](https://github.com/kike-canaries/android-hpma115s0) you can connect to your device via Bluetooth and record mobile captures and save tracks on your sdcard. Also you can share these tracks to CanAirIO network. If you want set your device for static station, please configure Wifi and CanAirIO API or InfluxDb server. (see below)

## [OPTIONAL] Set WiFi, CanAirIO API and custom InfluxDb configs via Bluetooth 

The current firmware (https://github.com/kike-canaries/esp32-hpma115s0/releases) supports setup WiFi crendentials, CanAirIO API or InfluxDb configs via Bluetooth for static statations. You can use the oficial [CanAirIO Android app](https://github.com/kike-canaries/android-hpma115s0) for send these settings to your device or you also can use [nRF Connect app](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) for the same tasks.

### Config via CanAirIO Android App:

Please connect your device via Bluetooth and in the settings section configure parameters like `Sample Time Interval` and `Station Name`. If you want configure our API cloud or a custom influxDb instance too.

### [Optional] Config via nRF Connect App:

#### WiFi Credentials

1. Start your sensor with last firmware (rev212)
2. Scan and connect to it with nRF connect App
3. Expand the GATT service item (Unknown Service, ends in aaf3)
4. Click on `upload button` on the `READ,WRITE` characteristic item (ends in ae02)
5. Change value type to `TEXT`
6. Put your credentials on `New Value` field, i.e. like this:
    ```json
    {"ssid":"YourWifiName","pass":"YourPassword"}
    ```
7. Click on `send` button.
8. On your serial messages your sensor will be log succesuful connection or on your display the wifi icon will be enable.

#### Device name (station name)

Repeat previous steps `1 to 6` but the payload for `dname` connection is for example:

```json
"{"dname":"PM25_Berlin_Pankow_E04"}"
```

#### InfluxDb config

Repeat previous steps `1 to 6` but the payload for `InfluxDb` connection is:

```json
"{"ifxdb":"","ifxip":"","ifxtg":""}"
```

the fields mean:
- **ifxdb**: InfluxDb database name
- **ifxip**: InflusDb hostname or ip
- **ifxtg**: Custom tags **(optional)**

##### Example:

```json
{"ifxdb":"database_name","ifxip":"hostname_or_ip","ifxtg":"zone=north,zone=south"}
```
#### Location config

Repeat previous steps `1 to 6` but the payload for `sensor location` for example is:

```json
"{"lat":52.53819,"lon":13.44024,"alt":220,"spd":34.5}"
```

#### InfluxDb payload

The current version send the next variables to InfluxDb:

```
pm25","pm10,"hum","tmp","lat","lng","alt","spd","stime"
```
- **pm25 and pm10**, from Honeywell sensor (is a average of `stime` samples)
- **hum and tmp**, humidity and temperature if you connect AM2320 to your ESP32
- **lat, lng, alt, spd**, variables that you already configured

## Device status vector

The current flags status is represented on one byte and it is returned on config:

``` java
bit_sensor  = 0;    // sensor fail/ok
bit_paired  = 1;    // bluetooth paired
bit_wan     = 2;    // internet access
bit_cloud   = 3;    // publish cloud
bit_code0   = 4;    // code bit 0
bit_code1   = 5;    // code bit 1
bit_code2   = 6;    // code bit 2
bit_code3   = 7;    // code bit 3

```

The error codes are represented on up four bits. Error code table:

``` java
ecode_sensor_ok          =   0;
ecode_sensor_read_fail   =   1;
ecode_sensor_timeout     =   2;
ecode_wifi_fail          =   3;
ecode_ifdb_write_fail    =   4;
ecode_ifdb_dns_fail      =   5;
ecode_json_parser_error  =   6;
ecode_invalid_config     =   7;
```

sample:

``` java
    00000011 -> sensor ok, device paired
    00001101 -> sensor ok, wan ok, ifxdb cloud ok
    01000101 -> sensor ok, wan ok, ifxdb write fail
```
---

<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/rev212.jpg" target="_blank"><img src="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/rev212.jpg" align="right" width="384" ></a>

## TODO
- [X] HPMA115S0 fixes and libraries tests
- [X] SSD1306 OLED display output (PM2.5 and PM10)
- [X] Basic output via Bluetooth LE GATT server
- [X] Gson output parser (for [Android client](https://github.com/kike-canaries/android-hpma115s0))
- [X] WeMOS OLED board supported
- [X] Heltec board supported
- [X] D1 MINI Kit OLED board supported
- [X] LaserCut box for D1Mini board
- [X] Config WiFi via Bluetooth
- [X] Config InfluxDb (Cronograf) via Bluetooth (without auth for now)
- [X] Config sample time via Bluetooth
- [X] GUI: bluetooth, wifi and cloud status icons 
- [ ] Real time clock or clock set via BT sync
- [ ] Timestamp for GPS sync
- [ ] Display graphs for PM2.5 and PM10
- [ ] ROM storage for offline issues

---

## Materials

Please for official materials and part list click on [wiki](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Hardware) or **full guide** on Hackster.io: [English](https://www.hackster.io/MetaKernel/canairio-citizen-network-for-air-quality-monitoring-bbf647) **|** [Spanish](https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a)


<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/materials.jpg" target="_blank"><img src="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/materials.jpg" align="right" width="384" ></a>

### Supported boards:

* [ESP32 Mini KIT (D1 mini compatible board)](http://bit.ly/2NLwtHK) (Recommended board)
* [ESP32 compatible board (WeMOS like)](http://bit.ly/2lMbWH6)
* [ESP32 Bluetooth WIFI Kit OLED](http://bit.ly/2neQI5f)

### Pollution sensors

* [Honeywell HPMA115S0](http://bit.ly/2II6647)

### Optional hardware

* [Battery module for ESP32 Mini KIT](http://bit.ly/2JSADuR) (Optional)
* [USB Power module board](http://bit.ly/2lHSKdr) (Optional)
* [Lipo Battery of 3.7v or similar](http://bit.ly/2KA3fdB) (Optional)

Hardware details on Hackster.io: [English](https://www.hackster.io/MetaKernel/canairio-citizen-network-for-air-quality-monitoring-bbf647) **|** [Spanish](https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a)

