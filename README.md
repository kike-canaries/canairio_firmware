# ESP32-HPMA115S0

**W A R N I N G :** Project in development

**Pollution sensor** builded with a `ESP32` module board and `HPMA115s0 Honeywell` dust sensor, interfaced with a [Android Client](https://github.com/kike-canaries/android-hpma115s0). You can download the last firmware version in [releases](https://github.com/kike-canaries/esp32-hpma115s0/releases) section.

## Dependencies

Please install before, [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE.

## Compiling and install

`pio run --target upload`

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
- [ ] Config firmware Characteristic
- [ ] Real time clock or clock set via BT sync
- [ ] Timestamp for GPS sync
- [ ] Gson output parser (for [Android client](https://github.com/kike-canaries/android-hpma115s0))
- [ ] Display graphs for PM2.5 and PM10
- [ ] ROM storage for offline issues

## Prototype

<a href="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/prototype.jpg"><img src="https://github.com/kike-canaries/esp32-hpma115s0/blob/master/images/prototype.jpg" align="left" width="1024" ></a>

<div>
____

</div>

## Materials

### Supported boards:

* [ESP32 compatible board (WeMOS like)](http://bit.ly/2lMbWH6)
* [ESP32 Mini KIT](http://bit.ly/2NLwtHK)
* [ESP32 Bluetooth WIFI Kit OLED](http://bit.ly/2neQI5f)

### Pollution sensors

* [Honeywell HPMA115S0](http://bit.ly/2pZPxYh)

### Optional hardware

* [Battery module for ESP32 Mini KIT](http://bit.ly/2JSADuR) (Optional)
* [USB Power module board](http://bit.ly/2lHSKdr) (Optional)
* [Lipo Battery of 3.7v or similar](http://bit.ly/2KA3fdB) (Optional)
