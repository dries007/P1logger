"""
This is the HC12Link firmware modeled in Python.
"""

import crcmod


CRC16 = crcmod.predefined.mkPredefinedCrcFun('crc-16')


def read_per_byte():
    """
    Mock for the UART data coming from the smart meter.
    """
    with open('./log.txt', 'rb') as f:
        while True:
            b = f.read(1)
            if len(b) == 0:
                return
            yield b


def read_line(inp):
    line = bytearray()
    while True:
        c = next(inp)
        line += c
        # print(c, line)
        if c == b'\n':
            break
    return bytes(line)


def single_packet(inp):
    # Packet Spec:
    # / is start of header. Header contains ID info + 2x CRLF.
    # Every line is an id and value in format "id(value)"
    # Meaning of id & value follows spec "DSMR 5.0.2 P1" and "eMUCs - P1".
    # ! is start of checksum. checksum = CRC16 over everything from / to !. Ends with CRLF.
    # Data is send every second.

    while True:
        line = read_line(inp)
        if line.startswith(b'/'):
            break

    crc = CRC16(line)
    print(hex(crc), line)

    while True:
        line = read_line(inp)
        if line.startswith(b'!'):
            break
        crc = CRC16(line, crc)
        print(hex(crc), line)

        if len(line) == 0:
            continue

        # sscanf(line, "")
        # print(line)

    crc = CRC16(b'!', crc)
    print(hex(crc), line)

    crc_footer = int(line[1:].decode('ascii'), 16)
    return crc == crc_footer


def main():
    inp = read_per_byte()
    while True:
        single_packet(inp)
        break


if __name__ == '__main__':
    main()
