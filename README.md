# eInk Crypto Currency

LilyGo EDP47 ESP32 Crypto currency News and Tracker

![preview](images/photo.jpg)
## Prerequisites

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

### WiFi Credentials:

Please first export your WiFi credentials like environment variables, for example

```python
export PIO_WIFI_PASSWORD='MyPassword'
export PIO_WIFI_SSID='MyWifiSSID'
```

### Config:

On `cryptos.h` choose your currencies in [this lines](https://github.com/hpsaturn/crypto-currency/blob/master/src/cryptos.h#L34-L38)  
On `platformio.ini` set power saving settings and temperature around the device

## Build and Upload

```bash
pio run --target upload
```

## TODO

- [x] improve TTGO eInk library to original library
- [x] remote OTA updates via PlatformIO and FOTA
- [x] new API for Crypto News (RSS)
- [x] diplay news feed and news link with a QR code
- [ ] set WiFi credentials via JSON on flash user partition
- [ ] run FOTA update without change WiFi credentials
- [ ] Bluetooth alternative for set preferences
- [ ] Easy firmware upload on first case use with [CanAirIO loader](https://github.com/hpsaturn/esp32-canairio-loader)

## Donations

- Via **Ethereum**:
- 0x1779cD3b85b6D8Cf1A5886B2CF5C53a0E072C108
- Be a patron: [Patreon](https://www.patreon.com/hpsaturn), [Github Sponsors](https://github.com/sponsors/hpsaturn) o [LiberaPay](https://liberapay.com/hpsaturn)
- Inviting me **a coffee**: [buymeacoffee](https://www.buymeacoffee.com/hpsaturn), [Sponsors](https://github.com/sponsors/hpsaturn?frequency=one-time)

<a href="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/ethereum_donation_address.png" target="_blank"><img src="https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/ethereum_donation_address.png" align="center_horizontal" width="180" ></a>

## Changelog

**20211106 rev145**

```
Added new News API and QR payload:

  - News API migrated to real server 
  - fix battery icon issue when is charging on USB 
  - first version with API for get QR link of news
  - added reboot method and improved global config
  - many memory improvements on QR allocation
``` 

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

## Credits

- Thanks to @techiesms by first idea and [original project](https://github.com/techiesms/) for Arduino IDE
- Thanks to @hacksics from some icons and fonts of project [HA dashboard project](https://github.com/hacksics/lilygo-t5-47-ha)

 
