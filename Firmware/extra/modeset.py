import serial

s = serial.Serial('/dev/ttyUSB1', timeout=1, baudrate=9600)

while True:
    print("Trying for OK...")
    s.write(b"AT\r\n")
    if s.readall() == b"OK\r\n":
        break

print("Ok")

s.write(b"AT+V\r\n")
print(s.readall())

s.write(b"AT+FU4\r\n")
print(s.readall())



