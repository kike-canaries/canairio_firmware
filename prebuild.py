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

# get runtime credentials and put them to compiler directive
env.Append(BUILD_FLAGS=[
    u'-DREVISION=' + revision + '',
    u'-DVERSION=\\"' + version + '\\"',
    u'-DFLAVOR=\\"' + flavor + '\\"',
    u'-DTARGET=\\"' + target + '\\"',
    u'-D'+ flavor + '=1',
    u'-I \"' + srcdir + '\"'
    ])

data = {
    "type":flavor, 
    "version":revision, 
    "host":"influxdb.canair.io",
    "port":8080,
    "bin":"/releases/" + target + "/canairio_" + flavor + "_rev" + revision + ".bin"
}

# build_flags = env.get("BUILD_FLAGS")
# print("NEW BUILD_FLAGS")
# print(build_flags)

output_path =  "releases/manifest/" + target

os.makedirs(output_path, 0o755, True)

output_manifiest = output_path + "/firmware_" + flavor + ".json"

with open(output_manifiest, 'w') as outfile:
    json.dump(data, outfile)

