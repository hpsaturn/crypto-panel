[![PlatformIO](https://github.com/hpsaturn/crypto-currency/workflows/PlatformIO/badge.svg)](https://github.com/hpsaturn/crypto-currency/actions/) ![ViewCount](https://views.whatilearened.today/views/github/hpsaturn/crypto-currency.svg) [![Liberapay Status](http://img.shields.io/liberapay/receives/hpsaturn.svg?logo=liberapay)](https://liberapay.com/hpsaturn)

# Crypto Panel

LilyGo EDP47 ESP32 cryptocurrency panel, News and Tracker.

![preview](images/photo.jpg)

## Features

- Panel installation and configuration via an easy [Web installer](https://hpsaturn.com/crypto-panel-installer/)
- Configuration via command line (CLI) with the console of the web installer
- Support hundreds of coins from [Coingecko API](https://api.coingecko.com/api/v3/coins/list?include_platform=false)
- Random cryptocurrencies news from Cointelegrah and others news portals
- The coin news are follow via QR code
- Base currency USD/EUR configurable
- Firmware update via automatic OTA updates
- Deep sleep configurable (default: 10min).
- Panel temperature ambient configurable (for improve colors)
- Included optional basic 3D-Print frame

## Firmware install

You able to install this firmware on only one click, without compiling nothing only using this [Web installer](https://hpsaturn.com/crypto-panel-installer/). Also you can follow the next [video guide](#demo) or follow the instruccions on the [Hackaday](https://hackaday.io/project/182527-crypto-panel) project page that using this web installer.

## Panel config

After the boot or first restart, please enter via serial console or terminal and please configure your settings, at least one WiFi network and 3 currencies. Type `help` and you should able to have the next menu:

```bash
ESP32WifiCLI Usage:

setSSID "YOUR SSID"   set the SSID into quotes
setPASW "YOUR PASW"   set the password into quotes
connect               save and connect to the network
list                  list all saved networks
select <number>       select the default AP (default: last saved)
mode <single/multi>   connection mode. Multi AP is a little slow
scan                  scan for available networks
status                print the current WiFi status
disconnect            disconnect from the network
delete "SSID"         remove saved network
help                  print this help

Crypto Panel Commands:

curAdd <crypto>       add one cryptocurrency
curList               list current saved cryptocurrencies
curDrop <crypto>      delete one cryptocurrency
setBase <base>        set base currency (USD/EUR)
setSleep <time>       config deep sleep time in minutes
setTemp <temperature> config the panel ambient temperature
reboot                perform a soft ESP32 reboot
help                  display this help menu
```

Please visit the project page in [Hackaday](https://hackaday.io/project/182527-crypto-news-eink-panel) for complete details.

## TODO

- [x] improve TTGO eInk library to original library
- [x] remote OTA updates via PlatformIO and FOTA
- [x] new API for Crypto News (RSS)
- [x] diplay news feed and news link with a QR code
- [x] cofigure the eInk panel via command line
- [x] run FOTA update without change WiFi credentials
- [x] New [Web installer](https://hpsaturn.com/crypto-panel-installer/)
- [x] NTP time zone via CLI
- [ ] Bluetooth alternative for set preferences

## Firmware build

Please install first [PlatformIO](http://platformio.org/) open source ecosystem for IoT development compatible with **Arduino** IDE and its command line tools (Windows, MacOs and Linux). Also, you may need to install [git](http://git-scm.com/) in your system.

```bash
pio run --target upload
```

## Demo

[![Crypto panel](https://raw.githubusercontent.com/hpsaturn/esp32-wifi-cli/master/images/cryptopanel_preview.jpg)](https://youtu.be/oyav6SvN870)

## Donations

- Via **Ethereum**:
- 0x1779cD3b85b6D8Cf1A5886B2CF5C53a0E072C108
- Be a patron: [Patreon](https://www.patreon.com/hpsaturn), [Github Sponsors](https://github.com/sponsors/hpsaturn) o [LiberaPay](https://liberapay.com/hpsaturn)
- Inviting me **a coffee**: [buymeacoffee](https://www.buymeacoffee.com/hpsaturn), [Sponsors](https://github.com/sponsors/hpsaturn?frequency=one-time)

<a href="images/ethereum_donation_address.png" target="_blank" style="padding-left: 40px" ><img src="images/ethereum_donation_address.png" width="180" ></a>


## Changelog

**20230827 rev160**
```
Multiple changes from rev150
  - disable OTA update (Only FOTA)
  - removed old NTP client library instead ESP32 internal client
  - reduced pem chain to only two certs
  - v0.1.3 Full URL support. Cuttpy API issues fixed
  - Espressif32 v6.3.2 support
  - freezes and updated core libraries (EPDIY, FOTA, NTP libs)
  - feature added: wakeup and setup via right button
  - new CLI settings: setSleep, setTemp and setBase
```

**20220713 rev153**
```
Easy firmware installer via Browser
  - added silent call for reduce verbose on serial
  - added web installer section and updated documentation
  - refactor load variables and delays on epdiy
```

**20220713 rev150**
```
Changed panel config via CLI
  - Added ESP32WifiCLI library
  - extended CLI to have a currencies configuration
```

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
- This project use the last version of [EPDIY driver](https://github.com/vroland/epdiy) of @vroland

 
