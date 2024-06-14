# build.py
# pre-build script, setting up build environment and fetch hal file for user's board

import sys
import os
import os.path
import requests
import json
from os.path import basename
from platformio import util
from SCons.Script import DefaultEnvironment

try:
    import configparser
except ImportError:
    import ConfigParser as configparser

# get platformio environment variables
env = DefaultEnvironment()
config = configparser.ConfigParser()
config.read("platformio.ini")

# get platformio source path
srcdir = env.get("PROJECTSRC_DIR")
flavor = env.get("PIOENV")
revision = config.get("common","revision")
version = config.get("common", "version")
target = config.get("common", "target")

# print ("environment:")
# print (env.Dump())

chipFamily = "ESP32"

if flavor == "ESP32C3" or flavor == "ESP32C3OIPLUS" or flavor == "ESP32C3LOLIN" or flavor == "ESP32C3SEEDX" or flavor == "AG_OPENAIR":
    chipFamily = "ESP32-C3"

if flavor == "ESP32S3" or flavor == "TTGO_T7S3":
    chipFamily = "ESP32-S3"

# get runtime credentials and put them to compiler directive
env.Append(BUILD_FLAGS=[
    u'-DREVISION=' + revision + '',
    u'-DVERSION=\\"' + version + '\\"',
    u'-DFLAVOR=\\"' + flavor + '\\"',
    u'-DFAMILY=\\"' + chipFamily + '\\"',
    u'-DTARGET=\\"' + target + '\\"',
    u'-D'+ flavor + '=1',
    u'-I \"' + srcdir + '\"'
    ])

manifest_fota = {
    "type":flavor, 
    "version":revision, 
    "host":"influxdb.canair.io",
    "port":8080,
    "bin":"/releases/" + target + "/canairio_" + flavor + "_rev" + revision + ".bin"
}

bin_path = "https://influxdb.canair.io/releases/webi/"+target+"/canairio_" + flavor + "_rev" + revision + "_merged.bin"

manifest_webi = {
  "name": "CanAirIO "+flavor,
  "version": revision,
  "new_install_prompt_erase": True,
  "funding_url": "https://liberapay.com/CanAirIO",
  "builds": [
    {
      "chipFamily": chipFamily,
      "parts": [
        { "path": bin_path, "offset": 0 }
      ]
    }
  ]
}

# build_flags = env.get("BUILD_FLAGS")
# print("NEW BUILD_FLAGS")
# print(build_flags)

manifest_dir =  "releases/manifest/" + target
manifest_webi_dir = "releases/manifest/webi/" + target

os.makedirs(manifest_dir, 0o755, True)
os.makedirs(manifest_webi_dir, 0o755, True)

manifest_fota_file = manifest_dir + "/firmware_" + flavor + ".json"

with open(manifest_fota_file, 'w') as outfile:
    json.dump(manifest_fota, outfile)

manifest_webi_file = manifest_webi_dir + "/firmware_" + flavor + ".json"

with open(manifest_webi_file, 'w') as outfile:
    json.dump(manifest_webi, outfile)

