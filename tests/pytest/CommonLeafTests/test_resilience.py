from xerxes_protocol import (
    Leaf,
    XerxesNetwork,
    MsgId,
    checksum
)
import time
import struct
import pytest
import random


__author__ = "theMladyPan"
__date__ = "2023-02-23"


# test resilience checksum errors
def test_resilience_ping(XN: XerxesNetwork, leaf: Leaf):
    """Tests resilience against network errors

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected
        XR (XerxesRoot): The root of the network
    """

    assert leaf.root.isPingLatest(leaf.ping())

    # craft a ping message with wrong checksum from scratch
    payload = bytes(MsgId.PING)
    source = 0x1e
    destination = 0x00
    chks = b"\x55"  # checksum (wrong on purpose)

    # send a message to the leaf, which will ignore it
    msg = b"\x01"  # SOH
    msg += (len(payload) + 5).to_bytes(1, "little")  # LEN
    msg += bytes(source)  # FROM
    msg += bytes(destination)  # DST
    msg += payload
    msg += chks  # payload checksum (wrong on purpose)

    # send the message over the network
    XN._s.write(msg)
    
    # wait for the leaf to process the message and read it, which should fail
    with pytest.raises(TimeoutError):
        XN.read_msg()


# test resilience against message length errors
def test_resilience_msg_length(XN: XerxesNetwork, leaf: Leaf):
    """Tests resilience against network errors

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected
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
    
    # wait for the leaf to process the message and read reply, which should fail
    with pytest.raises(TimeoutError):
        XN.read_msg()

    # test if the leaf is still connected to the network
    assert leaf.root.isPingLatest(leaf.ping())  


# test resilience against network overload
def test_resilience_network_overload(XN: XerxesNetwork, leaf: Leaf):
    """Tests resilience against network overload.

    Send a lot of gibberish to the network and see if the leaf is still connected.  

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected
        leaf (Leaf):        The leaf to test
    """
    
    # send > 100kiB of random data
    num_bytes = 100 * 1024
    for i in range(num_bytes):
        # send a random byte
        XN._s.write(struct.pack("B", int(random.random() * 1000 % 256)))
        if i % 100 == 0:
            # from time to time send a SOH so the leaf can start processing the invalid message
            XN._s.write(b"\x01")  # SOH

    # wait for the watchdog to kick in
    time.sleep(.1)

    # test if the leaf is still connected to the network
    leaf.root.isPingLatest(leaf.ping())