# ESC/POS Printer Library

A Library to control an ESC printer by an ESP32 via Bluetooth BLE

At the moment I don't have an Arduino with Bluetooth BLE, but it could easily be made compatible in the future.

## Requirements

An ESP32 board with Bluetooth BLE and an ESC Bluetooth BLE thermal printer, knowing the UUID of the service and the characteristic.

## Tested

- Chinese ESC/POS printer called "BlueTooth Printer" & ESP32-S2 super mini, ESP32-C3

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/GjCb8tO_vno/0.jpg)](https://www.youtube.com/watch?v=GjCb8tO_vno)

## How convert image

Using the image2cpp project to generate the byte array

https://javl.github.io/image2cpp/


We upload the image and change the configuration that appears in step 2

![step 2](https://github.com/rnrobles/esc-thermal-printer-ble/blob/main/images/image1.png?raw=true)

 
In step 4 we select plain bytes and press the Generate Code button

![step 4](https://github.com/rnrobles/esc-thermal-printer-ble/blob/main/images/image2.png?raw=true)

we copy only the generated array and add it to our .ino in the logo array


## If the library was useful to you
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/A0A3RVNSG)

## License

You may use this work under the terms of either the MIT License or the GNU General Public License (GPL) Version 3


### Keywords

thermal, micro, receipt, printer, serial, epson, arduino


## Based on
https://github.com/cranties/escposprinter

