# ESP32-HPMA115S0 (CanAirIO sensor)

<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/collage.jpg" target="_blank"><img src="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/collage.jpg" align="right" width="384" ></a>

**W A R N I N G :** Project in development

<a href="https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter" target="_blank"><img src="https://github.com/kike-canaries/android-hpma115s0/blob/master/assets/googleplay/gplayicon.png" align="left" width="128" ></a>

Citizen science project with mobile and fixed sensors for measuring air quality (PM 2.5) using low-cost sensors and smartphones. Built with a `ESP32` module board and `HPMA115s0 Honeywell` dust sensor, interfaced with an [CanAirIO Android client app](https://github.com/kike-canaries/android-hpma115s0).

**Full guide (Hackster.io):** [English](https://www.hackster.io/MetaKernel/canairio-citizen-network-for-air-quality-monitoring-bbf647) **|** [Spanish](https://www.hackster.io/114723/canairio-red-ciudadana-para-monitoreo-de-calidad-del-aire-96f79a)

## Software Dependencies

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

## Compiling and installing

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
You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section. 

### Troubleshooting

If you have some issues with Bluetooth library internals, please upgrade all frameworks and tools on PlatformIO:

```
pio update
sudo pio upgrade
pio run -t clean
rm -rf .pioenvs .piolibdeps
pio run --target upload
```

## WiFi and InfluxDb configs via Bluetooth [OPTIONAL]

The current firmware [rev212](https://github.com/kike-canaries/esp32-hpma115s0/releases/tag/rev212) supports set WiFi crendentials and InfluxDb config via Bluetooth. The [CanAirIO Android app](https://github.com/kike-canaries/android-hpma115s0) does not support it yet, for now you can use [nRF Connect app](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)

### Wifi config

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

### InfluxDb config

Repeat previous steps `1 to 6` but the payload for `InfluxDb` connection is:

```json
"{"ifxdb":"","ifxip":"","ifxid":"","ifxtg":""}"
```

the fields mean:
- **ifxdb**: InfluxDb database name
- **ifxip**: InflusDb hostname or ip
- **ifxid**: Sensor ID or sensor name
- **ifxtg**: Custom tags **(optional)**

#### Example:

```json
{"ifxdb":"database_name","ifxip":"hostname_or_ip","ifxid":"sensor_station_name","ifxtg":"zone=north,zone=south"}
```
### Location config

Repeat previous steps `1 to 6` but the payload for `sensor location` for example is:

```json
"{"lat":52.53819,"lon":13.44024,"alt":220,"spd":34.5}"
```

## InfluxDb payload

The current version send the next variables to InfluxDb:

```
pm25","pm10,"hum","tmp","lat","lng","alt","spd","stime"
```
- **pm25 and pm10**, from Honeywell sensor (is a average of `stime` samples)
- **hum and tmp**, humidity and temperature if you connect AM2320 to your ESP32
- **lat, lng, alt, spd**, variables that you already configured

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

