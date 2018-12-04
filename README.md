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
 - Draganboard 410c should be connected to Bluetooth speakers

## Application Installation
### QCA4020 Build
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

### DraganBoard 410c Setup
#### Install NodeJS
- Execute the following command in the command line to setup the NodeJS base version.
```sh
$ curl -sL https://deb.nodesource.com/setup_9.x | sudo -E bash – 
```
- Install NodeJS by executing the following command.
```
$> sudo apt-get install nodejs
```
- Install process manager module by executing the following command
```
$> npm install pm2 -g
```
#### Install Dependencies
- Run the setup script to install the dependencies. Please, find the script in the application’s root path. (~/Dragon_410c)
```
    $> ./setup.sh
```
#### Adding Songs
Please, move to below mentioned folder path
```
cd /demo2/qca-iiot-music-festival/songs/
```
You will see 4 folders
```
1. edm
2. hiphop
3. pop
4. rock
```
Please, add n number of songs with mp3 format. The application will shuffle among the songs and it will play.

## Hardware Setup
### QCA4020 PIR Sensor
Jumper settings for PIR:

![PIR jumper setup](images/qca4020_enable_PIR.png "Jumper Setup for QCA4020 to enable PIR")

### BLE Bulb
Connect Bulb to the power supply, and Rename the BLE name of bulb to the “HOME-BLB”
- Install Playbulb mobile app (any Android or IOS devices can be used). 
 - Opening the app will list the bulbs.
 - Connect to each bulb and by clicking on that bulb will move to next screen.
 - Click settings icon (gear icon) will show the Product Rename. 
 - Enter “PIR-20-MSCD” and click “Rename Confirm”  button to change the name.
 - Repeat this steps for all the 5 bulbs.
## Configuration Changes
Below is the Configuration which you can find in the below path open it if you want to edit it.
```sh
cd demo2/qca-iiot-music-festival/iiot-music-festival/config/config.json.
```
It contains 3 parts
```
SERIAL_COMM
AWS_IOT_CLIENT
APP_DB_CONFIG
{
        "SERIAL_COMM":{
                "serial_port":"/dev/ttyUSB1",
                "start_4020_app":"4 4 1",
                "start_music":"4 4 3",
                "stop_music": "4 4 2",
                "duration_in_sec" : "3",
                "frequency_threshold" : "3"
        },
        "AWS_IOT_CLIENT":{
                "host" : "a11hq9a0r6cqjs-ats.iot.eu-west-1.amazonaws.com",
                "rootCAPath" : "root-CA.crt",
                "certificatePath" : "QCA4020_MUSIC_FTV.cert.pem",
                "privateKeyPath" : "QCA4020_MUSIC_FTV.private.key",
                "clientId" : "QCA_Music_410c_Client",
                "topic" : "pir_timestamp",
                "mode" : "publish"
        },
        "APP_DB_CONFIG" :{
                "dbConfig" : {
                        "dialect": "sqlite3",
                        "schemaName": "music_festival",
                        "connection": {
                                "filename": "./demo2.db"
                        }
                },
                "port" : "8000"
        }
```
We need to change the following things, 
- serial_port - usb port name which we noted in earlier step.
- duration_in_sec – duration to observe number of movements.
- frequency_threshold – number of movements need to detected to change the color of LED.

Note: If any changes done on SERIAL_COMM part of the below configuration, changing the USB cables need to restart the QCA4020 board and make sure the serial_port need to be updated with correct USB port name and need to restart the NodeJS application (Steps mentioned below)
## Start Web APP
 - Please, move to below mentioned folder path
```
$ cd ~/Dragon_410c/qca-iiot-music-festival/iiot-music-festival/
```
- Execute the below command to start the application.
```
$ ./start.sh
```
- Execute the below command to stop the application.
```
$ ./stop.sh
```
Once, you start the application, wait for 3 minutes maximum to make QCA4020 to connect with all the bulbs. (Once it is connected, all the bulbs will blink in white colour every 5 seconds). In between if any bulb got disconnected, it will try to auto connect.

Use the below URL in the browser to start the application.
http://localhost:8000


