import serial

uart = serial.Serial("/dev/ttyUSB0", baudrate=115200)

with open('log.txt', 'wb') as f:
    while True:
        f.write(uart.readline())
        print('.', end='')
