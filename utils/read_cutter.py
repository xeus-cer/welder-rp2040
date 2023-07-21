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
from serial import Serial
import time
import logging

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
    default=100, 
    help="interval in milliseconds between reads, default is 100ms"
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

# add tuple of arguments to args (length, offset)
parser.add_argument(
    "-l",
    "--length",
    metavar="LENGTH",
    type=int,
    help="length of the tuple in MM",
    required=True
)

parser.add_argument(
    "-o",
    "--offset",
    metavar="OFFSET",
    type=int,
    help="offset of the tuple in MM",
    required=True
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
    exit_val = 0

    # set offset first because setting length will trigger a cut
    leaf.dv1 = args.offset
    # set length and start cutting
    leaf.dv0 = args.length

    while True:
        try:
            encoder_value = leaf.sv0
            pLength = leaf.dv0
            pOffset = leaf.dv1
            status = leaf.device_status
            print(
                f"Encoder value: {encoder_value}, Status: {status}, pLength[{pLength}], pOffset[{pOffset}]." + " " * 10,
                end="\r"
            )
            if args.history: 
                print()
            time.sleep(args.interval / 1000)
        except KeyboardInterrupt:
            print("Exiting...")
            break
        except TimeoutError:
            log.warning(f"TimeoutError while reading from {leaf}")
        except Exception as e:
            log.error(f"Exception {e} while reading from {leaf}")
            exit_val = 1
            break
    port.close()
    sys.exit(exit_val)
    
