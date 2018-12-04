import serial
import sys
import json

def serial_write(command):
    with open('config/config.json') as f:
        config = json.load(f)
    se = serial.Serial(config["SERIAL_COMM"]["serial_port"], 115200)
    if(command):
        se.write(command + "\r\n")
    se.close()

if __name__ == '__main__':
    if (len(sys.argv) > 1):
        serial_write(sys.argv[1])
    else:
        print("Provide the command to write to serial port")

