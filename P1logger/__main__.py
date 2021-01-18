"""
Copyright (c) 2020 Dries007
This code is licensed under MIT license (see LICENSE.txt for details)
"""

import os
import argparse

from . import P1logger

args = argparse.ArgumentParser()
args.add_argument("-p", "--port", default=os.environ.get("P1_PORT", "/dev/ttyUSB0"))
args.add_argument("-i", "--influx", default=os.environ.get("P1_INFLUX", "influxdb://localhost:8086/p1log"))


if __name__ == '__main__':
    P1logger(**args.parse_args().__dict__).run()
