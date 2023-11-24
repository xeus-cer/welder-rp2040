#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Module Name: Read Cutter
Description: This script performs reads and writes to the Xerxes Cutter device, 
                and prints the values in a tight loop. Use Ctrl+C to exit.
Author: theMladyPan
Version: 1.0
Date: 2023-05-15
"""

import os
import sys
import argparse
from serial import Serial, serialutil
import time
import logging

from xerxes_protocol import XerxesRoot, XerxesNetwork, Leaf, DebugSerial

# parse arguments
parser = argparse.ArgumentParser(
    description="Read process values from Xerxes Cutter device and print them in tight loop. Use Ctrl+C to exit."
)
parser.add_argument(
    "-a",
    "--address",
    metavar="ADDR",
    required=False,
    type=int,
    default=0,
    help="address of Xerxes Cutter device, default is 0",
)
parser.add_argument(
    "output",
    metavar="OUTPUT",
    type=int,
    help="output number to set",
)
# add argument whether to use debug serial or not
parser.add_argument(
    "-d",
    "--debug",
    action="store_true",
    help="use debug serial, default is False",
)
parser.add_argument(
    "-t",
    "--timeout",
    metavar="TIMEOUT",
    required=False,
    type=float,
    default=0.02,
    help="timeout in seconds for serial communication, default is 0.02s",
)
parser.add_argument(
    "-p",
    "--port",
    metavar="PORT",
    required=False,
    type=str,
    default="/dev/ttyUSB0",
    help="port on which Xerxes Cutter device is listening, default is /dev/ttyUSB0",
)

args = parser.parse_args()

try:
    if args.debug:
        level = logging.DEBUG
        port = DebugSerial(args.port)
    else:
        level = logging.INFO
        port = Serial(args.port, timeout=args.timeout)
except serialutil.SerialException:
    # print to stderr and exit
    print(f"Error: Could not open port {args.port}", file=sys.stderr)
    sys.exit(1)

logging.basicConfig(
    level=level, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
log = logging.getLogger(__name__)

# create XerxesNetwork object
XN = XerxesNetwork(port)
XN.init()

# create XerxesRoot object
XR = XerxesRoot(0xFE, XN)

# create Leaf object
leaf = Leaf(args.address, XR)
log.debug(f"Leaf parameters: {dir(leaf)}")
log.debug(f"Ping: {leaf.ping()}")

if __name__ == "__main__":
    exit_val = 0

    try:
        log.debug(f"Setting output to {args.output}")
        leaf.dv0 = args.output
        log.debug(f"leaf.dv0 = {leaf.dv0}")
    except TimeoutError:
        log.warning(f"TimeoutError while reading from {leaf}")
    except Exception as e:
        log.error(f"Exception {e} while reading from {leaf}")
        exit_val = 1

    port.close()
    sys.exit(exit_val)
