from xerxes_protocol.ids import MsgId
from xerxes_protocol.hierarchy.root import XerxesRoot
from xerxes_protocol.hierarchy.leaves.leaf import Leaf
from xerxes_protocol.network import Addr

import struct
import time
import pytest


__author__ = "theMladyPan"
__date__ = "2023-02-23"


def test_sleep_callback(leaf: Leaf):
    """Put the leaf to sleep for .5s and test the callback"""

    # duration in microseconds
    duration_us = 500_000  # .5s
    
    # put the leaf to sleep
    leaf.sleep(duration_us)

    with pytest.raises(TimeoutError):
        # try to read ping device after it has been put to sleep - should fail
        leaf.root.isPingLatest(leaf.ping())
    
    # wait for the leaf to wake up
    time.sleep(duration_us / 1_000_000)

    # try to read ping device after sleep
    assert leaf.root.isPingLatest(leaf.ping())


