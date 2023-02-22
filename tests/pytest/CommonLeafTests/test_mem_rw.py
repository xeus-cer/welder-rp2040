from xerxes_protocol import (
    Leaf,
    WriteErrorReadOnly
)
import math
import time
import pytest


__author__ = "theMladyPan"
__date__ = "2023-02-22"


def test_try_write_non_volatile_mem(cleanLeaf: Leaf):
    """Writes 4 bytes to register gain_pv0 and 4 bytes to register offset_pv0 and checks their retention"""

    # check if the values are not the same as the ones we want to write
    assert cleanLeaf.gain_pv0 != math.pi
    assert cleanLeaf.offset_pv0 != math.e

    # write the values
    cleanLeaf.gain_pv0 = math.pi
    cleanLeaf.offset_pv0 = math.e

    # reset to see if the values were written
    cleanLeaf.reset_soft()

    # wait for the reset to finish
    time.sleep(.5)

    # check if the values were written correctly with a tolerance of 1e-6
    assert cleanLeaf.gain_pv0 == pytest.approx(math.pi, 1e-6)
    assert cleanLeaf.offset_pv0 == pytest.approx(math.e, 1e-6)


def test_try_write_volatile_mem(cleanLeaf: Leaf):
    """Writes 4 bytes to register dv0 and 4 bytes to register memory_lock and checks their retention"""
    
    # check if the values are not the same as the ones we want to write
    cleanLeaf.dv0 != 0x12345678
    cleanLeaf.memory_lock != 0x87654321

    # write the values
    cleanLeaf.dv0 = 0x12345678
    cleanLeaf.memory_lock = 0x87654321

    # check if the values were written correctly
    assert cleanLeaf.dv0 == 0x12345678
    assert cleanLeaf.memory_lock == 0x87654321

    # reset to see if the values voided
    cleanLeaf.reset_soft()
    time.sleep(.5)

    # check if the values were reset
    assert cleanLeaf.dv0 != 0x12345678
    assert cleanLeaf.memory_lock != 0x87654321


def test_try_write_read_only(cleanLeaf: Leaf):
    """Write to a read only register and check if the attempt failed
    
    For this test to work, the leaf must be connected to the network. See MemoryReadOnly for more info.
    """

    assert cleanLeaf.device_uid != 0

    # try to write to the register, should fail

    with pytest.raises(WriteErrorReadOnly):
        cleanLeaf.device_uid = 0
