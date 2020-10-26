## CanAirIO Linux - MacOS installer

## Usage

For the firmware install script help updated please run here:

``` bash
./install.sh help
```

## Binaries and tools

You can download this installer in [releases](https://github.com/kike-canaries/canairio_firmware/releases) section or [build](#Build) it and get this directory, get binaries and some python tools dependencies.

## Installer parameters

```python
usage: ./install.sh [binary]
usage: ./install.sh [command] [binary] [option] [option]

Examples

Install via default USB port:

./install.sh canairio_d1mini_20190323rev273.bin

Install via WiFi:

./install.sh ota canairio_xxx.bin
./install.sh ota canairio_xxx.bin 192.168.1.10

Install via USB:

./install.sh usb canairio_xxx.bin
./install.sh usb canairio_xxx.bin /dev/ttyUSB3
./install.sh usb canairio_xxx.bin /dev/ttyUSB3 1500000

You can download old binaries files in relases:
https://github.com/kike-canaries/canairio_firmware/releases

Support channel: Telegram and Forum links on https://canair.io
```

## Output samples

### USB update

```python
./install.sh usb canairio_TTGO_T7_rev654_20201026.bin /dev/ttyUSB0 1500000

##########################################
firmware: canairio_TTGO_T7_rev654_20201026.bin
update via USB: /dev/ttyUSB0
speed: 1500000
##########################################

esptool.py v2.8
Serial port /dev/ttyUSB0
Connecting....
Uploading stub...
Hard resetting via RTS pin...
```

### WiFi update

```python
./install.sh ota canairio_TTGO_T7_OTA_rev654_20201026.bin 192.168.178.68

###################################################
firmware: canairio_TTGO_T7_OTA_rev654_20201026.bin
update via IP: 192.168.178.68
speed: 
###################################################

11:19:24 [DEBUG]: Options: {'timeout': 10, 'esp_ip': '192.168.178.68', 'host_port': 34610, 'image': 'canairio_TTGO_T7_OTA_rev654_20201026.bin', 'host_ip': '0.0.0.0', 'auth': 'CanAirIO', 'esp_port': 3232, 'spiffs': False, 'debug': True, 'progress': True}
11:19:24 [INFO]: Starting on 0.0.0.0:34610
11:19:24 [INFO]: Upload size: 1659120
Sending invitation to 192.168.178.68 
Authenticating...OK
11:19:24 [INFO]: Waiting for device...
Uploading: [============================================================] 100% Done...

11:20:20 [INFO]: Waiting for result...
11:20:21 [INFO]: Result: OK
11:20:21 [INFO]: Success
```

# Build

For rebuild all binaries, tools and this directory please first execute in the root of the CanAirIO firmware project:

``` bash
./build all && ./build installer
```

For more options for build the installer:

``` bash
./build help
```
