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

from xerxes_protocol import XerxesRoot, XerxesNetwork, Leaf, DebugSerial

# parse arguments
parser = argparse.ArgumentParser(
    description="Read process values from XeUS Welder device and print them in tight loop. Use Ctrl+C to exit."
)
parser.add_argument(
    "-a",
    "--address",
    metavar="ADDR",
    required=False,
    type=int,
    default=-1,
    help="address of Xerxes Cutter device, default is 0",
)
parser.add_argument(
    "-i",
    "--interval",
    metavar="MS",
    required=False,
    type=int,
    default=100,
    help="interval in milliseconds between reads, default is 100ms",
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
    "--loglevel",
    metavar="LOGLEVEL",
    required=False,
    type=str,
    default="INFO",
    help="log level, default is INFO",
)
parser.add_argument(
    "-w",
    "--weld_time_ms",
    metavar="WELD_TIME_MS",
    type=int,
    help="weld time in milliseconds",
    required=True,
)

# whether to show history or not in output formating
parser.add_argument(
    "--history", help="show history in output", action="store_true"
)

args = parser.parse_args()

level = logging.DEBUG if args.debug else logging.getLevelName(args.loglevel)
logging.basicConfig(
    level=level, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
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
XR = XerxesRoot(0xFE, XN)
leaf = None
if args.address < 0:
    for i in range(32):
        leaf = Leaf(i, XR)
        try:
            ping = leaf.ping()
            log.info(f"Found device at address {i}, ping: {ping}")
            break
        except TimeoutError:
            log.debug(f"No device at address {i}")
            leaf = None
        except Exception as e:
            log.error(f"Exception {e} while reading from {leaf}")
else:
    # create Leaf object
    leaf = Leaf(args.address, XR)
    ping = leaf.ping()
    log.debug(f"Found device at address {args.address}, ping: {ping}")
if not leaf:
    log.error("No device found, exiting...")
    sys.exit(1)
log.debug(f"Leaf parameters: {dir(leaf)}")

if __name__ == "__main__":
    exit_val = 0

    # set length and start cutting
    leaf.dv0 = args.weld_time_ms

    while True:
        try:
            weldingFor = leaf.pv0
            pWeldTimeMs = leaf.dv0
            status = leaf.device_status
            print(
                f"Welding for: {weldingFor:.3f}s, Status: {status}, Weld time: {pWeldTimeMs}ms"
                + " " * 10,
                end="\r",
            )
            if args.history:
                print()
            if not status:
                print()
                break
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
