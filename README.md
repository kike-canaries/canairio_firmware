# ESP32-HPMA115S0


**W A R N I N G :** Project in development

**Pollution sensor** builded with a `ESP32` module board and `HPMA115s0 Honeywell` dust sensor, interfaced with a [Android Client](https://github.com/kike-canaries/android-hpma115s0). You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section.

## Dependencies

Please install before, [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE.


---
<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/collage.jpg" target="_blank"><img src="https://play.google.com/store/apps/details?id=hpsaturn.pollutionreporter" align="right" width="512" ></a>
---

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

**NOTE:** for each board the `Rx/Tx` of sensor maybe change.

## Troubleshooting

If you have some issues with Bluetooth library internals, please upgrade all frameworks and tools on PlatformIO:

```
pio update
sudo pio upgrade
pio run -t clean
rm -rf .pioenvs .piolibdeps
pio run --target upload
```

## TODO
- [X] HPMA115S0 fixes and libraries tests
- [X] SSD1306 OLED display output (PM2.5 and PM10)
- [X] Basic output via Bluetooth LE GATT server
- [X] Gson output parser (for [Android client](https://github.com/kike-canaries/android-hpma115s0))
- [X] WeMOS OLED board supported
- [X] Heltec board supported
- [X] D1 MINI Kit OLED board supported
- [X] LaserCut box for D1Mini board
- [ ] Config firmware Characteristic
- [ ] Real time clock or clock set via BT sync
- [ ] Timestamp for GPS sync
- [ ] Display graphs for PM2.5 and PM10
- [ ] ROM storage for offline issues

## Materials

Please for official materials and part list click on [wiki](https://github.com/kike-canaries/esp32-hpma115s0/wiki/Official-Hardware)

### Supported boards:

* [ESP32 Mini KIT](http://bit.ly/2NLwtHK) (Recommendend board)
* [ESP32 compatible board (WeMOS like)](http://bit.ly/2lMbWH6)
* [ESP32 Bluetooth WIFI Kit OLED](http://bit.ly/2neQI5f)

### Pollution sensors

* [Honeywell HPMA115S0](http://bit.ly/2pZPxYh)

### Optional hardware

* [Battery module for ESP32 Mini KIT](http://bit.ly/2JSADuR) (Optional)
* [USB Power module board](http://bit.ly/2lHSKdr) (Optional)
* [Lipo Battery of 3.7v or similar](http://bit.ly/2KA3fdB) (Optional)
