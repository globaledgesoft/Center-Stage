# Center Stage


This project is designed to use the proximity sensor on the QCA4020 development board to gauge crowd engagement at the center stage of a music festival. This sensor value can be used to change color patterns on BLE light bulbs to create an engaging atmosphere at the venue. The QCA4020 Development board is also programmed to interface with the DragonBoard™ 410c from Arrow Electronics which is designed to act as a smart gateway that runs a node.js based web application and notifies the event organizers when the crowd engagement drops.

Below are the list of connected devices:
 - QCA4020 (Inbuilt PIR sensor)
 - DragonBoard™ 410c (Connected via Serial to QCA4020)
 - Playbulbs’s BLE Bulb - 5 no.s (Connected via BLE)

QCA4020 will automatically scan and connect to Playbulbs’s BLE Bulb based on Blub name and Charecteristic UUID. Will also handles disconnections automatically.

# QCA 4020 Board Application Installation

## Pre-requisite
 - Playbulbs’s BLE Bulb must be named as “PIR-20-MSCD”. Can be done through playbulb's mobile app.
 - Setup the QCA402x sdk (Provided in this repo)
 - Setup the gcc-arm-none-eabi toolchain and proper environment variable, to compile the source
 - Python 2.7 or more has to be installed on windows system to run flashing program
 - Install the QDLoader in windows to setup the flashtool
 - Flashed Music_Demo's Binary on the board

## Application Installation
 
 - Go to the `QCA4020_sdk/target/quartz/demo/Music_Demo/build/gcc` from CMD/Terminal
 - For `Linux`, run below command to build the binary
```sh
user@user:~$ cd QCA4020_sdk/target/quartz/demo/build/gcc
user@user:gcc$ make prepare
user@user:gcc$ make
```
 - For `Windows`, run below command to build binary
```sh
C:/ >cd /[path to application directory]/build/gcc
C:/[path to application directory]/build/gcc > build.bat
```
 - The above steps will generate the binary in `output` folder in `build/gcc` folder
 - Now add jumper to pin `J34(1,2)` for setting the board to the `flash mode`
 - Goto `output` directory on your console, and type below command in windows to flash the binary on the board,
```sh
C:/[path to application directory]/build/gcc/output > py -2 QCA402x_sdk/target/build/tools/flash/qflash.py –comm=<USB Port Number>
```
`NOTE: Command must be run inside build/gcc/output directory presents in Application directory.`


## Hardware Setup
### QCA4020 PIR Sensor
Jumper settings for PIR:

![PIR jumper setup](images/qca4020_enable_PIR.jpg "Jumper Setup for QCA4020 to enable PIR")

### BLE Bulb
Connect Bulb to the power supply, and Rename the BLE name of bulb to the “HOME-BLB”
- Install Playbulb mobile app (any Android or IOS devices can be used). 
 - Opening the app will list the bulbs.
 - Connect to each bulb and by clicking on that bulb will move to next screen.
 - Click settings icon (gear icon) will show the Product Rename. 
 - Enter “PIR-20-MSCD” and click “Rename Confirm”  button to change the name.
 - Repeat this steps for all the 5 bulbs.
```sh

