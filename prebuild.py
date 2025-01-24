# Author: @hpsaturn - 2021
# prebuild.py
# pre-build script, setting up build environment and fetch flavor board information

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
    installer_path = 'releases/installer/eInkCrypto_installer/'
    print("Uploading {0} to OTA server..".format(firmware_name))
    # subprocess.call(["./build", "clean"])
    subprocess.call(["cp", "%s" % firmware_path, "%s" % installer_path])
    subprocess.call(["./build", "ota"])

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
    env.Replace(PROGNAME="eInkCrypto_%s_rev%s" % (flavor,revision), UPLOADCMD=publish_firmware)

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

chipFamily = "ESP32"

if flavor == "ESP32C3" or flavor == "ESP32C3OIPLUS" or flavor == "ESP32C3LOLIN" or flavor == "ESP32C3SEEDX" or flavor == "AG_OPENAIR":
    chipFamily = "ESP32-C3"

if flavor == "ESP32S3" or flavor == "TTGO_T7S3":
    chipFamily = "ESP32-S3"

manifest_fota = {
    "type":flavor, 
    "version":revision, 
    "host":"influxdb.canair.io",
    "port":8080,
    "bin":"/releases/" + target + "/eInkCrypto_" + flavor + "_rev" + revision + ".bin"
}

bin_path = "https://influxdb.canair.io/releases/webi/"+target+"/eInkCrypto_" + flavor + "_rev" + revision + "_merged.bin"

manifest_webi = {
  "name": "CryptoPanel "+flavor,
  "version": revision,
  "new_install_prompt_erase": True,
  "funding_url": "https://liberapay.com/hpsaturn",
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

