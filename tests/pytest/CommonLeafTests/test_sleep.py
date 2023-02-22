from xerxes_protocol.ids import MsgId
from xerxes_protocol.hierarchy.root import XerxesRoot
from xerxes_protocol.hierarchy.leaves.leaf import Leaf
from xerxes_protocol.network import Addr
from xerxes_protocol import PROTOCOL_VERSION_MAJOR, PROTOCOL_VERSION_MINOR
import struct
import time


def test_sleep_callback(XR: XerxesRoot):
    """Put the leaf to sleep for 5s"""
    # create a leaf object with address 0 and the root object XR
    l = Leaf(Addr(0), XR)

    # duration in microseconds
    duration_us = 1_000_000  # 1s
    # create a 4 byte duration value
    duration_bytes = struct.pack("I", duration_us)
    # send a message to the leaf, which puts it to sleep for 5s
    XR.send_msg(0, bytes(MsgId.SLEEP) + duration_bytes)
    try:
        # try to read ping device during sleep
        reply = l.ping()
        # should fail
        assert False
    except TimeoutError:
        # should fail
        assert True
    
    # wait for the leaf to wake up
    time.sleep(duration_us / 1_000_000)
    # try to read ping device after sleep
    reply = l.ping()
    # should not fail
    assert reply is not None
    assert reply.v_maj == PROTOCOL_VERSION_MAJOR
    assert reply.v_min == PROTOCOL_VERSION_MINOR


