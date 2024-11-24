import serial

s = serial.Serial('/dev/ttyUSB0', timeout=2, baudrate=1200)

while True:
    # s.write(b"AB")
    print(s.read())
