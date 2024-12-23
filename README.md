| Supported Targets | ESP32 | ESP32-S3 |
|-------------------|-------|----------|

Bluetooth: https://bluetooth.com

BLE 2M
PHY: https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/nimble/ble_phy/phy_prph/tutorial/Ble_Phy_Peripheral_Example_Walkthrough.md

Deep Sleep: https://github.com/espressif/esp-idf/blob/master/examples/system/deep_sleep/README.md

Real Time Stats: https://github.com/espressif/esp-idf/blob/master/examples/system/freertos/real_time_stats/README.md

OTA: https://github.com/espressif/esp-idf/blob/master/examples/system/ota/README.md

BQ27742-G1: https://www.ti.com/lit/ds/symlink/bq27742-g1.pdf
https://www.ti.com/lit/ug/sluuax0c/sluuax0c.pdf?ts=1718563280100
https://www.ti.com/lit/an/slua467b/slua467b.pdf?ts=1722969672876&ref_url=https%253A%252F%252Fwww.google.com%252F

### Performance improvements:

##### ✅ Increase BaudRate of UART Comm

##### ✅ Use 2M PHY for BLE Comm

##### Configure WiFi Comm properly to get maximum throughput in data transfer

### Plan of Action:

##### ✅ Increase BaudRate of UART Comm

##### ✅ Detect Baud Rate of UART Comm

##### ✅ Detect Mac Address of WiFi softAP / WiFi STA / BLE

##### ✅ BLE Comm: 2M PHY Support

##### ✅ WiFi Comm: then resolve IP using mDNS

##### ✅ Resolve cors error in http server

##### ✅ Button Press Actions

##### ✅ Stop scanning when BLE / WiFi comm disconnects

##### ✅ Turn ON Fans while Scanning

##### Integrate with BMS I2C

##### [⚠️ Need to provide correct battery soc info] ✅ Add Battery, Device Info Service to BLE Gatt Server and Advertise Battery Service

##### Deep Sleep Feature

##### OTA

##### Diagnostics API

##### Display

##### Display Mac address / connection string of selected comm mode

##### [⚠️ Undone, Performance test is required] BT / WiFi controller host tasks to CPU Core 1 (using menuconfig & tasks_common)

### Changes:

##### [⚠️ Antenna Ports config is remaining] ✅ Initialise the reader upon power on (Boot firmware, Baud rate, protocol config, antenna ports)

Get connected antennas - works with closed loop antennas only
Set connected antennas read / write power

### Programming R&D

##### create perfect sdkconfig

# RFID Reader

Two Way Communication (Client <=> Reader <=> rfid module)

                       Client                 <-- 
                      /  |  \
                    /    |    \
                  /      |      \
                /        |        \
            Wifi        BLE         USB       <-- Communication modes
          /     \        |        /
       http     mqtt     |       /            <-- Communication methods over WiFi
          \__      \     |      /
              \__ __\    |     /
                  comm interface              <-- Generic Communication Interface
                    /          \
                  prphs       RF              <-- Command Type

| Header | Command Type |
|--------|--------------|
| 00     | Reader       |
| FF     | RFID         |

| Description               | Header | Data Len | Command Code | Status Code | Data                                | Options                                                                      |
|---------------------------|--------|----------|--------------|-------------|-------------------------------------|------------------------------------------------------------------------------|
| Get Comm_Mode             | 00     | 00       | 01           |             |                                     |                                                                              |
| ↳ Response                | 00     | 01       | 01           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Comm_Mode             | 00     | 01       | 01           |             | 00                                  |                                                                              |
| ↳ Response                | 00     | 01       | 01           | 00 00       | 00                                  | 00 - BLE, 01 - WiFi AP, 02 - WiFi STA, 03 USB                                |
|                           |        |          |              |             |                                     |                                                                              |
| Get Time                  | 00     | 00       | 02           |             |                                     |                                                                              |
| ↳ Response                | 00     | 04       | 02           | 00 00       | 3B A7 8F 24                         |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Time                  | 00     | 04       | 02           |             | 3B A7 8F 24                         |                                                                              |
| ↳ Response                | 00     | 04       | 02           | 00 00       | 3B A7 8F 24                         |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Network SSID          | 00     | 00       | 03           |             |                                     |                                                                              |
|                           | 00     | 09       | 03           | 00 00       | 48 6F 6D 65 20 57 69 46 69          |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Network SSID          | 00     | 09       | 03           |             | 48 6F 6D 65 20 57 69 46 69          |                                                                              |
|                           | 00     | 09       | 03           | 00 00       | 48 6F 6D 65 20 57 69 46 69          |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Network Password      | 00     | 00       | 04           |             |                                     |                                                                              |
|                           | 00     | 08       | 04           | 00 00       | 70 61 73 73 77 6F 72 64             |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Network Password      | 00     | 08       | 04           |             | 70 61 73 73 77 6F 72 64             |                                                                              |
|                           | 00     | 08       | 04           | 00 00       | 70 61 73 73 77 6F 72 64             |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Notification (Button) | 00     | 00       | 05           |             |                                     |                                                                              |
|                           | 00     | 01       | 05           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Notification (Button) | 00     | 01       | 05           |             | 00                                  | 00 - No notify, 01 - LED, 02 - Beep, 03 - LED + Beep                         |
|                           | 00     | 01       | 05           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Notification (Alert)  | 00     | 00       | 06           |             |                                     |                                                                              |
|                           | 00     | 01       | 06           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Notification (Alert)  | 00     | 01       | 06           |             | 00                                  | 00 - No notify, 01 - LED, 02 - Beep, 03 - LED + Beep                         |
|                           | 00     | 01       | 06           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Display Mode          | 00     | 00       | 07           |             |                                     |                                                                              |
|                           | 00     | 01       | 07           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Tag Display Mode      | 00     | 01       | 07           |             | 00                                  | 00 - HEX, 01 - ASCII                                                         |
|                           | 00     | 01       | 07           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Device Name           | 00     | 00       | 08           |             |                                     |                                                                              |
|                           | 00     | 0C       | 08           | 00 00       | 52 66 69 64 52 65 61 64 65 72 2D 31 |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Device Name           | 00     | 0C       | 08           |             | 52 66 69 64 52 65 61 64 65 72 2D 31 |                                                                              |
|                           | 00     | 0C       | 08           | 00 00       | 52 66 69 64 52 65 61 64 65 72 2D 31 |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Debug Mode            | 00     | 00       | 0D           |             |                                     |                                                                              |
|                           | 00     | 01       | 0D           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Debug Mode            | 00     | 01       | 0D           | 01          |                                     | 00 - False, 01 - True                                                        |
|                           | 00     | 01       | 0D           | 00 00       | 01                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Error Display Mode    | 00     | 00       | 0A           |             |                                     |                                                                              |
|                           | 00     | 01       | 0A           | 00 00       | 01                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set Error Display Mode    | 00     | 01       | 0A           |             | 00                                  | 00 - NO, 01 - YES                                                            |
|                           | 00     | 01       | 0A           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get BTN Click Action      | 00     | 00       | B1           |             |                                     |                                                                              |
|                           | 00     | 01       | B1           | 00 00       | 00                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set BTN Click Action      | 00     | 01       | B1           |             | FF                                  | 00 - NO Action, FF - NOTIFY                                                  |
|                           | 00     | 01       | B1           | 00 00       | FF                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get BTN Hold Action       | 00     | 00       | B2           |             |                                     |                                                                              |
|                           | 00     | 05       | B2           | 00 00       | FF 00 03 1D 0C                      |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set BTN Hold Action       | 00     | 05       | B2           |             | FF 00 03 1D 0C                      | Set custom command upon button hold                                          |
|                           | 00     | 05       | B2           | 00 00       | FF 00 03 1D 0C                      |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get BTN Release Action    | 00     | 00       | B3           |             |                                     |                                                                              |
|                           | 00     | 05       | B3           | 00 00       | FF 00 03 1D 0C                      |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Set BTN Release Action    | 00     | 05       | B3           |             | FF 00 03 1D 0C                      |                                                                              |
|                           | 00     | 05       | B3           | 00 00       | FF 00 03 1D 0C                      |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Battery SOC           | 00     | 00       | BA           |             |                                     |                                                                              |
|                           | 00     | 01       | BA           | 00 00       | 64                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Battery SOH           | 00     | 00       | BB           |             |                                     |                                                                              |
|                           | 00     | 01       | BB           | 00 00       | 64                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Get Battery Cycle Count   | 00     | 00       | BC           |             |                                     |                                                                              |
|                           | 00     | 01       | BC           | 00 00       | 08                                  |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Factory Reset             | 00     | 00       | FD           |             |                                     |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| Reboot                    | 00     | 00       | FE           |             |                                     |                                                                              |
|                           |        |          |              |             |                                     |                                                                              |
| OTA Upgrade               | 00     | 07       | FF           |             | 31 2E 30 2E 30 2B 31                | Hex representation of Version string which user wants to upgrade / downgrade |

| component            | status | comments                                                              |
|----------------------|--------|-----------------------------------------------------------------------|
| app state dispatcher | ✅      |                                                                       |
| ble app              | ✅      |                                                                       |
| comm if              |        | need to group the message before sending, need to add message parsing |
| http server          | ✅      |                                                                       |
| usb app              |        |                                                                       |
| wifi app             | ✅      |                                                                       |
| ota                  |        |                                                                       |
| battery              |        | need to test whether integration is correct or not                    |
| buttons              | ✅      | fast single clicks are not recorded                                   |
| buzzer               | ✅      |                                                                       |
| fan                  | ✅      |                                                                       |
| leds                 | ✅      |                                                                       |
| display              |        |                                                                       |
| settings             | ✅      | in new mcu device name is not shown for the first time                |
| silion sim 7200      | ✅      |                                                                       |
| uart                 | ✅      |                                                                       |
| www                  |        | need to make ez-controller functional                                 |

set baud rate to 115200: FF 14 AA 4D 6F 64 75 6C 65 74 65 63 68 AA 40 06 01 00 01 C2 00 B4 BB 9D EB
set baud rate to 921600: FF 14 AA 4D 6F 64 75 6C 65 74 65 63 68 AA 40 06 01 00 0E 10 00 0F BB 79 9F 
