#/bin/bash

showHelp () {
  echo ""
  echo "usage: install.sh [canairio_xxx.bin]"
  echo ""
  echo "example:"
  echo "install.sh canairio_d1mini_20190323rev273.bin"
  echo ""
  echo "install via wifi:"
  echo "usage: install.sh ota [canairio_xxx.bin]"
  echo ""
  echo "You can download a bin file in relases:"
  echo "https://github.com/kike-canaries/esp32-hpma115s0/releases"
  echo ""
}

flashOTA () {
 ./espota.py --port=3232 --auth=CanAirIO --debug --progress -i 'CanAirIO.local' -f $1
}

flash () {
  ./esptool.py --chip esp32 --port "/dev/ttyUSB0" --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 system/bootloader_dio_40m.bin 0x8000 system/partitions.bin 0xe000 system/boot_app0.bin 0x10000 $1
}



if [ "$1" = "" ]; then
  showHelp
else
  case "$1" in
    help)
      showHelp
      ;;

    ota)
      flashOTA $2
      ;;

    *)
      flash $1
      ;;
  esac
fi
