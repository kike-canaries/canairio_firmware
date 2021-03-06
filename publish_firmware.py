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

# get bintray user credentials from ota key file
bintray_config = {k: v for k, v in config.items("bintray")}
user = bintray_config.get("user")
repository = bintray_config.get("repository")
apitoken = bintray_config.get("api_token")
package = bintray_config.get("package")

# get bintray upload parameters from platformio environment
version = config.get("common", "release_version")

# put bintray user credentials to platformio environment
env.Replace(BINTRAY_USER=user)
env.Replace(BINTRAY_REPO=repository)
env.Replace(BINTRAY_API_TOKEN=apitoken)

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
