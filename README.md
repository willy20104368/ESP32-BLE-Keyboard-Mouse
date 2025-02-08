# ESP32-BLE-Keyboard-Mouse
* This project implements a BLE Keyboard & Mouse based on ESP-IDF.
* In this project, we used a joystick to control mouse movement, scrolling, and clicking, and five buttons to type all 26 characters (a–z).

****
## Configure the Project

* `button.h & button.c` These files define the functions of the buttons, including ISR, debouncing, and mapping button events to corresponding characters.
* `joystick.h & joystick.c` These files define the functions of the joystick, including reading value changes from the joystick and debouncing the button.
* `main.c` This file shows how to use the HID (which allows you to connect to a Windows PC and act as a keyboard and mouse). The RTC pin is set to wake up the device from deep sleep if it remains idle for too long.
* `others` Please refer to this [link](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/ble/ble_hid_device_demo#esp-idf-ble-hid-example).

****
## How to use?
* First, see this [link](https://github.com/espressif/esp-idf) to set up the ESP-IDF development environment
* Then, run `idf.py -p PORT flash monitor` to build, flash and monitor the project.
