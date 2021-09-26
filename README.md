# eInk Crypto Currency

LilyGo EDP47 Crypto currency Tracker

![preview](images/preview.gif)
## Prerequisites

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

### WiFi Credentials:

Please first export your WiFi credentials like environment variables, for example

```python
export PIO_WIFI_PASSWORD='MyPassword'
export PIO_WIFI_SSID='MyWifiSSID'
```

### Config:

On `cryptos.h` choose your currencies.  
On `platformio.ini` you can change the power saving settings

## Build and Upload

```bash
pio run --target upload
```

## Changelog


**20210926 rev100**

```
Added WDT, status queue msg, improved UI:

  - Fast main update on each partial refresh
  - Columns aligned to RIGHT
  - New error msgs on WiFi lost and API fails
  - Final msg on status bar is joined with data render
  - Battery level improvements for USB/Battery modes
``` 

**20210925 rev089**

```
Added OTA updates (local and remote)

  - Python tool for deployment via PlatformIO
  - Improved OTAHandler from CanAir.IO project
  - Added UI feedback when new OTA is arrived
``` 

**20210924 rev076**

```
Migration from LilyGo forked library to vroland/epdiy repo

  - works fine with USB, with battery sometimes fails
  - added ntpdate clock sync for show the last time update
  - added missing fonts. Improved static fields refresh
  - added UI tools from github.com/hacksics/lilygo-t5-47-ha
  - working again all with new driver, ~20 seg on init
``` 

**20210922 rev039 (First version)**

```
  Added support to PlatformIO and improved code.

  - improved speed adding a RTOS task for eINK static tasks
  - refactored and improved PlatformIO ini file with heredity
  - added a basic wifi manager from CanAir.IO project
  - added battery level and reset detection for full refresh
  - deep sleep improvement with full refresh after x boots
  - Original code: https://github.com/techiesms/
  
``` 















