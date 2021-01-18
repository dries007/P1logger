"""
Copyright (c) 2020 Dries007
This code is licensed under MIT license (see LICENSE.txt for details)
"""

import crcmod
import serial
import re
import influxdb
import traceback

from .obis import EMUCS_V1_4

OBJECT_REGEX = re.compile(r"^(\d)-(\d):(\d+)\.(\d+)\.(\d+)")


class P1logger:
    """
    Data logger for P1 port on digital power/gas/... meters.
    """
    def __init__(self, port: str, influx: str) -> None:
        self.serial = serial.Serial(port, 115200)
        self.crc16 = crcmod.predefined.mkPredefinedCrcFun('crc-16')
        self.influx = influxdb.client.InfluxDBClient.from_dsn(influx)
        # noinspection PyProtectedMember
        self.influx.create_database(self.influx._database)

    def read_packet(self):
        # For full specs of packets, documents in readme.
        #   eMUCs – P1 v1.4
        #   DSMR 5.0.2 P1
        #   IEC 62056-21 (OBIS)

        # Packet Spec:
        # / is start of header. Header contains ID info + 2x CRLF.
        # Every line is an id and value in format "id(value)"
        # Meaning of id & value follows spec "DSMR 5.0.2 P1" and "eMUCs - P1".
        # ! is start of checksum. checksum = CRC16 over everything from / to !. Ends with CRLF.
        # Data is send every second.

        # Buffer entire message (for CRC)

        line = self.serial.readline()
        while not line.startswith(b'/'):
            line = self.serial.readline()

        header = line[1:].strip().decode('ascii')
        crc = self.crc16(line)

        # Eat newline after header
        line = self.serial.readline()
        crc = self.crc16(line, crc)
        assert line == b"\r\n"

        # Now comes data
        fields = {}
        human_fields = {}
        line = self.serial.readline()
        while not line.startswith(b'!'):
            crc = self.crc16(line, crc)
            raw = line.strip().decode('ascii')
            m = OBJECT_REGEX.match(raw)
            a, b, c, d, e = map(int, m.groups())
            # b = meter ID in case of extra meters, not used for OBIS lookup.
            f, human_name, *_ = EMUCS_V1_4[(a, c, d, e)]
            # Full prefix
            prefix = raw[:m.end()]
            # Value without ( and )
            raw_value = raw[m.end()+1:-1]

            value = f(raw_value)

            fields[prefix] = value
            if human_name:
                human_fields[human_name] = value

            # print(human_name, value, prefix, raw_value, raw, sep='\t')

            line = self.serial.readline()

        # Append final ! to CRC, then compare and ignore packet if it was bad.
        crc = self.crc16(b'!', crc)
        crc_footer = int(line[1:].decode('ascii'), 16)

        crc_ok = crc == crc_footer
        time = fields.get("0-0:1.0.0", None)
        if time:
            del fields["0-0:1.0.0"]
            del human_fields["time"]

        fields["crc_ok"] = crc_ok
        # print(f"{time}: {fields} CRC={crc_ok}")
        # print(f"{time}: {human_fields} CRC={crc_ok}")
        self.influx.write_points([
            {
                "measurement": "p1",
                "time": time,
                "fields": fields,
                "tags": {
                    "header": header,
                },
            },
            {
                "measurement": "p1_human",
                "time": time,
                "fields": human_fields,
                "tags": {
                    "header": header,
                },
            }
        ])
        print(f"{time}: +{human_fields['power_used']:.1f}kW -{human_fields['power_injected']:.1f}kW CRC={crc_ok}")

    def run(self):
        while True:
            try:
                self.read_packet()
            except Exception as e:
                if isinstance(e, KeyboardInterrupt):
                    return
                traceback.print_exc()
