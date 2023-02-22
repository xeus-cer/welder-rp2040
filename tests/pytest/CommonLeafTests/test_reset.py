from xerxes_protocol import (
    MemoryVolatile,
    Leaf,
    XerxesPingReply,
    XerxesRoot,
    Addr,
    MsgId
)

import time
import struct
from pprint import pprint as print
import pytest


__author__ = "theMladyPan"
__date__ = "2023-02-22"


def test_reset_soft(leaf: Leaf):
    leaf.reset_soft()
    with pytest.raises(TimeoutError):
        leaf.ping()
    
    # wait for the leaf to reboot
    time.sleep(.5)

    # check if leaf is still connected to the network
    assert leaf.root.isPingLatest(leaf.ping())


def test_reset_hard(leaf: Leaf):
    leaf.reset_hard()
    with pytest.raises(TimeoutError):
        leaf.ping()
    
    # wait for the leaf to reboot
    time.sleep(.5)

    # check if leaf is still connected to the network
    assert leaf.root.isPingLatest(leaf.ping())


def test_hard_reset_fail(cleanLeaf: Leaf, XR: XerxesRoot):
    """Test if the hard reset fails when the magic bytes are wrong."""

    # change PV2 offset to pi=3.14
    cleanLeaf.offset_pv2 = 3.14

    # write the wrong magic bytes to the memory lock register
    mem_lock_offset = MemoryVolatile.memory_lock.elem_addr

    invalid_magic_bytes = struct.pack("I", 0xAA55AA55)
    cleanLeaf.write_reg(mem_lock_offset, invalid_magic_bytes)
    
    # send a message to the leaf, which resets it
    with pytest.raises(TimeoutError):
        reply = cleanLeaf.exchange(bytes(MsgId.RESET_HARD))
        assert reply is not None

        # reset should fail because of wrong magic bytes, so the reply should be NOK
        assert reply.message_id == MsgId.ACK_NOK


    # send a ping to the leaf, which should still be alive
    assert XR.isPingLatest(cleanLeaf.ping())

    # check that the PV2 offset is still approximately pi
    leaf = Leaf(Addr(0), XR)
    assert leaf.offset_pv2 == pytest.approx(3.14, abs=1e-6)