#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Module Name: tier_test.py
Description: Test script
Author: theMladyPan
Version: 1.0
Date: 2023-06-15
"""

import argparse
from serial import Serial
import logging
import time


from xerxes_protocol import (
    XerxesRoot,
    XerxesNetwork,
    Leaf,
    DebugSerial
)

# parse arguments
parser = argparse.ArgumentParser(description='Read process values from Xerxes Cutter device and print them in tight loop. Use Ctrl+C to exit.')
parser.add_argument(
    '-a', 
    "--address", 
    metavar='ADDR', 
    required=False, 
    type=int, default=0, 
    help='address of Xerxes Cutter device, default is 0'
)
parser.add_argument(
    "-i", 
    '--interval', 
    metavar='MS', 
    required=False, 
    type=int, 
    default=10, 
    help="interval in milliseconds between reads, default is 10ms"
)
parser.add_argument(
    "-p", 
    '--port', 
    metavar='PORT', 
    required=False, 
    type=str, 
    default="/dev/ttyUSB0", 
    help='port on which Xerxes Cutter device is listening, default is /dev/ttyUSB0'
)
# add argument whether to use debug serial or not
parser.add_argument(
    "-d", 
    "--debug", 
    action="store_true", 
    help="use debug serial, default is False"
)
parser.add_argument(
    "-t",
    "--timeout",
    metavar="TIMEOUT",
    required=False,
    type=float,
    default=0.02,
    help="timeout in seconds for serial communication, default is 0.02s"
)
parser.add_argument(
    "--loglevel",
    metavar="LOGLEVEL",
    required=False,
    type=str,
    default="INFO",
    help="log level, default is INFO"
)


# whether to show history or not in output formating
parser.add_argument(
    "--history",
    help="show history in output",
    action="store_true"
)

args = parser.parse_args()

if args.debug:
    level = logging.DEBUG
else:
    level = logging.getLevelName(args.loglevel)

logging.basicConfig(
    level=level,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
log = logging.getLogger(__name__)

# create XerxesNetwork object
if args.debug:
    port = DebugSerial(args.port)
else:
    port = Serial(args.port, timeout=args.timeout)
XN = XerxesNetwork(port)
XN.init()

# create XerxesRoot object
XR = XerxesRoot(0xfe, XN)

# create Leaf object
leaf = Leaf(args.address, XR)
log.debug(f"Leaf parameters: {dir(leaf)}")

if __name__ == "__main__":
    try:
        log.info(f"Ping: {leaf.ping()}")
        leaf.dv0 = 1

        while leaf.device_status or leaf.dv0:
            print(
                f"Status={leaf.device_status}, dv0={leaf.dv0}", 
                end="\r"
            )
            time.sleep(args.interval / 1000)
    except TimeoutError:
        log.error("TimeoutError")
    except KeyboardInterrupt:
        log.info("Exiting...")
    finally:
        log.info("Closing port")
        port.close()
        exit(0)