# ESP32-HPMA115S0

ESP32 with HPMA115s0 Honeywell dust sensor with a WeMOS like board and [Android Client](https://github.com/kike-canaries/android-hpma115s0)

## Dependencies

Please install before, [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE.

## Compiling and install

`pio run --target upload`

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

## Materials

* [ESP32 compatible board (WeMOS)](http://bit.ly/2lMbWH6)
* [Honeywell HPMA115S0](http://bit.ly/2pZPxYh)
* [USB Power module board](http://bit.ly/2lHSKdr) (Optional)
* [Lipo Battery of 3.7v or similar](http://bit.ly/2KA3fdB) (Optional)
