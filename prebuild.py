# build.py
# pre-build script, setting up build environment and fetch hal file for user's board

import sys
import os
import os.path
import requests
import json
import subprocess

from os.path import basename
from platformio import util
from SCons.Script import DefaultEnvironment

try:
    import configparser
except ImportError:
    import ConfigParser as configparser

def publish_firmware(source, target, env):
    firmware_path = str(source[0])
    firmware_name = basename(firmware_path)
    installer_path = 'releases/installer/canairio_installer/'
    print("Uploading {0} to OTA server..".format(firmware_name))
    # subprocess.call(["./build", "clean"])
    subprocess.call(["cp", "%s" % firmware_path, "%s" % installer_path])
    subprocess.call(["./build", "otatrigger"])

# get platformio environment variables
env = DefaultEnvironment()
config = configparser.ConfigParser()
config.read("platformio.ini")

# get platformio source path
srcdir = env.get("PROJECTSRC_DIR")

revision = config.get("common","revision")
version = config.get("common", "version")
target = config.get("common", "target")
flavor = str(env.get("PIOENV"))

if flavor.find('OTA') != -1:
    flavor = flavor.replace('OTA','')
    # Custom upload command and program name
    env.Replace(PROGNAME="canairio_%s_rev%s" % (flavor,revision), UPLOADCMD=publish_firmware)

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

output_path =  "releases/manifest/" + target

os.makedirs(output_path, 0o755, True)

output_manifiest = output_path + "/firmware_" + flavor + ".json"

with open(output_manifiest, 'w') as outfile:
    json.dump(data, outfile)


