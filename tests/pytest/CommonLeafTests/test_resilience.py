from xerxes_protocol.hierarchy.leaves.leaf import Leaf
from xerxes_protocol.network import XerxesNetwork, Addr, XerxesPingReply, XerxesMessage, checksum
from xerxes_protocol.ids import MsgIdMixin, MsgId
from xerxes_protocol.hierarchy.root import XerxesRoot
from xerxes_protocol import PROTOCOL_VERSION_MAJOR, PROTOCOL_VERSION_MINOR
import time
import struct
import pytest
import random


# test resilience checksum errors
def test_resilience_ping(XN: XerxesNetwork, XR: XerxesRoot, leaf: Leaf):
    """Tests resilience against network errors

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected
        XR (XerxesRoot): The root of the network
    """

    assert XR.isPingLatest(leaf.ping())

    # craft a ping message
    payload = bytes(MsgId.PING)
    source = 0x1e
    destination = 0x00
    chks = b"\x55"  # checksum (wrong on purpose)

    # send a message to the leaf, which resets it
    msg = b"\x01"  # SOH
    msg += (len(payload) + 5).to_bytes(1, "little")  # LEN
    msg += bytes(source)  # FROM
    msg += bytes(destination)  # DST
    msg += payload
    msg += chks  # payload checksum (wrong on purpose)

    XN._s.write(msg)
    
    try:
        XN.read_msg()
        # should fail
        assert False
    except TimeoutError:
        pass


# test resilience against message length errors
def test_resilience_msg_length(XN: XerxesNetwork, XR: XerxesRoot, leaf: Leaf):
    """Tests resilience against network errors

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected
        XR (XerxesRoot): The root of the network
        leaf (Leaf): The leaf to test
    """

    # craft a ping message
    payload = bytes(MsgId.PING)
    source = 0x1e
    destination = 0x00
    
    # send a message to the leaf, which resets it
    msg = b"\x01"  # SOH
    msg += b"\xFF"  # LEN (wrong on purpose)
    msg += bytes(source)  # FROM
    msg += bytes(destination)  # DST
    msg += payload
    msg += checksum(msg)  # message checksum

    XN._s.write(msg)
    
    try:
        XN.read_msg()
        # should fail
        assert False
    except TimeoutError:
        pass

    # test if the leaf is still connected to the network
    ping = leaf.ping()
    assert ping is not None
    assert ping.v_maj == PROTOCOL_VERSION_MAJOR
    assert ping.v_min == PROTOCOL_VERSION_MINOR   


# test resilience against network overload
def test_resilience_network_overload(XN: XerxesNetwork, XR: XerxesRoot, leaf: Leaf):
    """Tests resilience against network overload.

    Send a lot of gibberish to the network and see if the leaf is still connected.  

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected
        XR (XerxesRoot):    The root of the network
        leaf (Leaf):        The leaf to test
    """

    # send 100kiB of random data
    num_bytes = 100 * 1024
    for i in range(num_bytes):
        # send a random byte
        XN._s.write(struct.pack("B", int(random.random() * 1000 % 256)))

    # wait for the watchdog to kick in
    time.sleep(.1)

    # test if the leaf is still connected to the network
    ping = leaf.ping()
    assert ping is not None
    assert ping.v_maj == PROTOCOL_VERSION_MAJOR 
    assert ping.v_min == PROTOCOL_VERSION_MINOR
