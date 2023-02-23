from xerxes_protocol import (
    Leaf,
    XerxesNetwork,
    MsgId,
    checksum,
    Addr
)
import time
import struct
import pytest
import random
import logging
_log = logging.getLogger(__name__)


__author__ = "theMladyPan"
__date__ = "2023-02-23"


# test resilience checksum errors
def test_resilience_ping(XN: XerxesNetwork, leaf: Leaf):
    """Tests resilience against network errors

    Args:
        XN (XerxesNetwork): The network on which the leaf is connected 
        leaf (Leaf): The leaf to test
    """

    assert leaf.root.isPingLatest(leaf.ping())

    # craft a ping message with wrong checksum from scratch
    payload = bytes(MsgId.PING)
    source = Addr(0x1e)
    destination = Addr(0x00)
    chks = b"\x55"  # checksum (wrong on purpose)

    # send a message to the leaf, which will ignore it
    msg = b"\x01"  # SOH
    msg += (len(payload) + 5).to_bytes(1, "little")  # LEN - 5 bytes for the header and checksum
    msg += bytes(source)  # FROM
    msg += bytes(destination)  # DST
    msg += payload
    msg += chks  # payload checksum (wrong on purpose)

    _log.info(f"Sending message: {msg.hex()} with wrong checksum")

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
    
    source = Addr(0x1e)
    destination = Addr(0x00)
    
    # send a message to the leaf, which resets it
    msg = b"\x01"  # SOH
    msg += b"\xFF"  # LEN (wrong on purpose)
    msg += bytes(source)  # FROM
    msg += bytes(destination)  # DST
    msg += payload
    msg += checksum(msg)  # message checksum

    _log.info(f"Sending message: {msg.hex()} with wrong length")
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
    
    # send 128kiB of random data
    num_bytes = 128 * 1024
    for i in range(int(num_bytes / 128)):
        # send a random chunk
        chunk_b = b"\x01"  # Start with SOH
        for j in range(128 - 1):
            chunk_b += struct.pack("B", int(random.random() * 1000 % 256))

        # send the chunk
        _log.info(f"Sending chunk {i+1} of {int(num_bytes / 128)}...")
        XN._s.write(chunk_b)

        # let the leaf process the message
        time.sleep(.02)

        # wait for the leaf to process the message and check if it is still connected
        assert leaf.root.isPingLatest(leaf.ping())

    # wait for the watchdog to kick in
    time.sleep(.1)

    # test if the leaf is still connected to the network
