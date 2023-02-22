
import pytest
from serial import Serial, SerialException
import os
import time
import struct
from xerxes_protocol import (
    Leaf,
    XerxesRoot,
    XerxesNetwork,
    Addr,
    DebugSerial
)


def get_serial_com() -> Serial:
    if os.name == "nt":
        # som na windows
        com = Serial(port="COM15", baudrate=115200, timeout=0.01)

    else:
        # on linux machine:
        com = DebugSerial(port="/dev/ttyUSB0", baudrate=115200, timeout=0.02)

    try:
        com.open()
    except SerialException:
        pass
    return com


def pytest_configure():
    pytest.com = get_serial_com()


@pytest.fixture
def XN() -> XerxesNetwork:
    """Prepare Xerxes network to test with.

    Returns:
        XerxesNetwork: Xerxes network to test with.
    """
    com: Serial
    com = pytest.com
    XN = XerxesNetwork(com)
    XN.init(timeout=0.01)
    return XN


@pytest.fixture(autouse=True)
def XR(XN: XerxesNetwork) -> XerxesRoot:
    """Prepare Xerxes root to test with.

    Args:
        XN (XerxesNetwork): Xerxes network to test with.

    Returns:
        XerxesRoot: Xerxes root to test with.
    """
    XR = XerxesRoot(0x1E, XN)
    return XR


@pytest.fixture
def leaf(XR: XerxesRoot) -> Leaf:
    """Prepare Xerxes leaf to test with.

    Args:
        XR (XerxesRoot): Xerxes root to test with

    Returns:
        Leaf: Xerxes leaf to test with.
    """
    leaf = Leaf(Addr(0), XR)

    # check if leaf is connected to the network first
    XR.isPingLatest(leaf.ping())

    return leaf


@pytest.fixture
def cleanLeaf(XR: XerxesRoot) -> Leaf:
    """Prepare Xerxes leaf to test with.

    Args:
        XR (XerxesRoot): Xerxes root to test with.

    Returns:
        Leaf: Xerxes leaf to test with.
    """
    leaf = Leaf(Addr(0), XR)

    # check if leaf is connected to the network first
    assert XR.isPingLatest(leaf.ping())

    # clean the leaf - reset it to factory default state    

    leaf.reset_hard()
    
    # wait for the leaf to reboot
    time.sleep(.5)

    # send a ping to the leaf
    assert XR.isPingLatest(leaf.ping())

    return leaf
