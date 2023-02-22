from xerxes_protocol import (
    PROTOCOL_VERSION_MAJOR,
    PROTOCOL_VERSION_MINOR,
    XerxesRoot,
    Leaf,
    MsgId,
    Addr,
    MAGIC_UNLOCK
)
import struct
import pytest
import time


@pytest.fixture
def leaf(XR: XerxesRoot):
    l = Leaf(Addr(0), XR)

    # check if leaf is connected to the network first
    ping = l.ping()
    assert ping is not None
    assert ping.v_maj == PROTOCOL_VERSION_MAJOR
    assert ping.v_min == PROTOCOL_VERSION_MINOR

    # reset the leaf to a factory state
    
    # unlock the memory of the leaf by writing the magic bytes to the memory lock register
    mem_lock_offset = 384
    magic_bytes = struct.pack("I", MAGIC_UNLOCK)
    l.write_reg(mem_lock_offset, magic_bytes)

    #  create a message id for the reset command
    msgid_reset = MsgId.RESET_HARD

    # send a message to the leaf, which resets it
    try:
        reply = l.exchange(bytes(msgid_reset))
        # should not return anything
        assert False
    except TimeoutError:
        pass  
        
    # wait for the leaf to reset and send a ping reply
    time.sleep(.2)

    ping = l.ping()
    assert ping is not None
    assert ping.v_maj == PROTOCOL_VERSION_MAJOR
    assert ping.v_min == PROTOCOL_VERSION_MINOR

    # assert config is 0
    config_offset = 40
    # read 1 byte
    reply = l.read_reg(config_offset, 1)
    assert reply is not None
    assert reply.payload == int(0).to_bytes(1, "little")

    return l


def test_sync(XR: XerxesRoot, leaf: Leaf):
    """
    Tests the sync method of the leaf

    Args:
        XR (XerxesRoot): The root of the network
        leaf (Leaf): The leaf to test
    """
    # last values should be 0
    pv_offset = 256
    # read 4 floats
    vals_raw = leaf.read_reg(pv_offset, 4*4)
    # convert to floats
    vals = struct.unpack("ffff", vals_raw.payload)[:4]
    assert not any(vals)

    XR.sync()
    # wait for sensor to update
    time.sleep(.2)

    # last values should not be 0
    vals_raw = leaf.read_reg(pv_offset, 4*4)
    # convert to floats
    vals = struct.unpack("ffff", vals_raw.payload)[:4]

    # check that at least one value is not 0
    assert any(vals)
