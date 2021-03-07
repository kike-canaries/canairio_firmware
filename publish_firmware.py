# build.py
# pre-build script, setting up build environment and fetch hal file for user's board

import sys
import os
import os.path
import requests
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

# check if ota key file is present in source directory
otakeyfile = os.path.join (srcdir, config.get("common", "otakeyfile"))
if os.path.isfile(otakeyfile) and os.access(otakeyfile, os.R_OK):
    print("Parsing OTA keys from " + otakeyfile)
else:
    sys.exit("Missing file " + otakeyfile + ", please create it! Aborting.")

mykeys = {}
# parse ota key file
with open(otakeyfile) as myfile:
    for line in myfile:
        key, value = line.partition("=")[::2]
        mykeys[key.strip()] = str(value).strip()

# usage of bintray: see https://github.com/r0oland/bintray-secure-ota

# get bintray user credentials from ota key file
user = mykeys["BINTRAY_USER"]
repository = mykeys["BINTRAY_REPO"]
apitoken = mykeys["BINTRAY_API_TOKEN"]
package = config.get("platformio","default_envs")

# get bintray upload parameters from platformio environment
version = config.get("common", "release_version")

# put bintray user credentials to platformio environment
env.Replace(BINTRAY_USER=user)
env.Replace(BINTRAY_REPO=repository)
env.Replace(BINTRAY_PACKAGE=package)
env.Replace(BINTRAY_API_TOKEN=apitoken)
env.Replace(VERSION=version)

# get runtime credentials and put them to compiler directive
env.Append(BUILD_FLAGS=[
    u'-DBINTRAY_USER=\\"' + mykeys["BINTRAY_USER"] + '\\"', 
    u'-DBINTRAY_REPO=\\"' + mykeys["BINTRAY_REPO"] + '\\"', 
    u'-DBINTRAY_PACKAGE=\\"' + package + '\\"',
    u'-DVERSION=\\"' + version + '\\"',
    u'-I \"' + srcdir + '\"'
    ])

# function for pushing new firmware to bintray storage using API
def publish_bintray(source, target, env):
    firmware_path = str(source[0])
    firmware_name = basename(firmware_path)
    url = "/".join([
        "https://api.bintray.com", "content",
        user, repository, package, version, firmware_name
    ])

    print("Uploading {0} to Bintray. Version: {1}".format(
        firmware_name, version))
    print(url)

    headers = {
        "Content-type": "application/octet-stream",
        "X-Bintray-Publish": "1",
        "X-Bintray-Override": "1"
    }

    r = None
    
    try:
        r = requests.put(url,
                         data=open(firmware_path, "rb"),
                         headers=headers,
                         auth=(user,apitoken))
        r.raise_for_status()
    except requests.exceptions.RequestException as e:
        sys.stderr.write("Failed to submit package: %s\n" %
                         ("%s\n%s" % (r.status_code, r.text) if r else str(e)))
        env.Exit(1)

    print("The firmware has been successfuly published at Bintray.com!")

# put build file name and upload command to platformio environment
env.Replace(
    PROGNAME="firmware_" + package + "_v%s" % version,
    UPLOADCMD=publish_bintray)
