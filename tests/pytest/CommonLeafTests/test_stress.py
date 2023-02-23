from xerxes_protocol import (
    Leaf, 
    MsgId,
    LeafConfig,
    MemoryVolatile
)
import time
import struct
import random


__author__ = "theMladyPan"
__date__ = "2023-02-23"


# stress test for reading registers 0-1024
def test_read_stress(cleanLeaf: Leaf):
    """Reads 1024 registers and prints them in fixed width format

    Args:
        XR (XerxesRoot): XerxesRoot instance
    """

    # read 1024 registers in chunk of 4 bytes, 1024/4 = 256
    for i in range(0, 256):
        # read 4 bytes from register i
        xm = cleanLeaf.read_reg(i * 4, 4)

        # check if message id is correct
        assert xm.message_id == MsgId.READ_REPLY
        assert len(xm.payload) == 4

        # unpack float
        val_f = struct.unpack("f", xm.payload)[0]

        # alteratively unpack unsigned int
        val_i = struct.unpack("I", xm.payload)[0]

        # print both in fixed width
        print(f"Reg# {i*4:3d}: {val_f:10.3f} {val_i:10d}")


def test_read_speed(cleanLeaf: Leaf):
    """Reads 1024 registers in a burst and calculates the speed in bytes per second"""

    start = time.perf_counter_ns()

    chunk = 196
    # read 1024 registers in chunk of 196 bytes, 1024/196 ~ 6
    for i in range(0, int(1024 / chunk)):
        xm = cleanLeaf.read_reg(i * chunk, chunk)
        assert xm.message_id == MsgId.READ_REPLY  # check if message id is correct
        assert len(xm.payload) == chunk  # check if payload is correct length

    end = time.perf_counter_ns()

    # calculate speed in bytes per second, 256 registers * 4 bytes per register = 1024 bytes
    # 1024 bytes / time in seconds
    speed = 1024 / ((end - start) / 1e9)  # bytes per second
    
    # print speed in kiB/s
    print(f"Read speed: {speed/1024:.2f} kiB/s")

    # check if speed is greater than 1 kiB/s, oh my god it's fast
    assert speed > 1024  # 1 kiB/s


def test_write_stress_cycle_time(cleanLeaf: Leaf):
    """Writes 25 random cycle times to desired_cycle_time_us and checks if they were set correctly"""
    
    for i in range(0, 25):
        # create a random interval in microseconds
        cycle_time_us = random.randint(100000, 300000)
        
        # write cycle time to register leaf
        cleanLeaf.desired_cycle_time_us = cycle_time_us

        # check if the cycle time was set correctly
        assert cleanLeaf.desired_cycle_time_us == cycle_time_us

        # wait for 100ms
        time.sleep(.2)


def test_read_mean_burst(cleanLeaf: Leaf):
    """Reads 4 registers in a burst and calculates the mean value of the first 4 bytes

    Args:
        cleanLeaf (Leaf): Leaf instance
    
    Asserts:
        mean (list): mean value of at least one register is greater than 0
    """

    cleanLeaf.device_config = LeafConfig.freeRun | LeafConfig.calcStat
    
    burst = 100
    mean = [0, 0, 0, 0]

    for i in range(0, burst):
        # read 4 floats from register mean_pv0
        xm = cleanLeaf.read_reg(MemoryVolatile.mean_pv0.elem_addr, 16)
        assert xm.message_id == MsgId.READ_REPLY  # check if message id is correct
        vals = struct.unpack("ffff", xm.payload)[:4]  # unpack 4 floats

        # add each value to the mean
        for i in range(0, 4):
            mean[i] += vals[i]
    
    # calculate mean
    for i in range(0, 4):
        mean[i] = mean[i] / burst
    
    # print mean values
    print(f"Mean values: {mean[0]:.3f} {mean[1]:.3f} {mean[2]:.3f} {mean[3]:.3f}")
    
    # check if at least one value is not zero
    assert any(mean)
