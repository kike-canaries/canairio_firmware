#/bin/bash

FIRMWARE=$1
OTAHOSTIP="'CanAirIO.local'"
USBPORT="/dev/ttyUSB0"
USBSPEED=115200

showHelp () {
  echo ""
  echo "usage: ./install.sh [binary]"
  echo "usage: ./install.sh [command] [binary] [option] [option]"
  echo ""
  echo "Examples"
  echo ""
  echo "Install via default USB port:"
  echo ""
  echo "./install.sh canairio_d1mini_20190323rev273.bin"
  echo ""
  echo "Install via WiFi:"
  echo ""
  echo "./install.sh ota canairio_xxx.bin"
  echo "./install.sh ota canairio_xxx.bin 192.168.1.10"
  echo ""
  echo "Install via USB:"
  echo ""
  echo "./install.sh usb canairio_xxx.bin"
  echo "./install.sh usb canairio_xxx.bin /dev/ttyUSB3"
  echo "./install.sh usb canairio_xxx.bin /dev/ttyUSB3 1500000"
  echo ""
  echo "You can download old binaries files in relases:"
  echo "https://github.com/kike-canaries/canairio_firmware/releases"
  echo ""
  echo "Support channel: Telegram and Forum links on https://canair.io"
  echo ""
}

flashOTA () {
 ./espota.py --port=3232 --auth=CanAirIO --debug --progress -i $2 -f $1
}

flash () {
  ./esptool.py --chip esp32 --port $2 --baud $3 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 system/bootloader_dio_40m.bin 0x8000 system/partitions.bin 0xe000 system/boot_app0.bin 0x10000 $1
}

printParams() {
  echo "update via $1: $2"
  echo "firmware: $3"
}

##########################################
#  M  A  I  N
##########################################

if [ "$1" = "" ]; then
  showHelp
  exit 1
fi

case "$1" in
  help)
    showHelp
    ;;

  ota)
    if [ "$3" != "" ]; then
      OTAHOSTIP="$3"
    fi
    FIRMWARE="$2"
    if [ ${FIRMWARE: -4} == ".bin" ]; then
      printParams "IP" $OTAHOSTIP $FIRMWARE
    else
      showHelp
    fi

    flashOTA "$2" "$OTAHOSTIP"

    ;;

  usb)
    if [ "$3" != ""]; then
      USBPORT="$3"
    fi
    if [ "$4" != ""]; then
      USBSPEED="$4"
    fi
    FIRMWARE="$2"
    if [ ${FIRMWARE: -4} == ".bin" ]; then
      printParams "USB" $USBPORT $FIRMWARE
    else
      showHelp
    fi

    flash "$2" "$USBPORT" "$USBSPEED"

    ;;

  *)
    if [ ${FIRMWARE: -4} == ".bin" ]; then
      printParams "USB" $USBPORT $FIRMWARE
      flash "$FIRMWARE" "$USBPORT" "$USBSPEED"
    else
      showHelp
    fi
    ;;
esac
