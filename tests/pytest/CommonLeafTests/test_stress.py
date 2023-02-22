from xerxes_protocol import (
    XerxesRoot, 
    Leaf, 
    Addr, 
    XerxesMessage,
    MsgIdMixin,
    MsgId,
    PROTOCOL_VERSION_MAJOR,
    PROTOCOL_VERSION_MINOR
)
import time, struct
import random
import pytest


@pytest.fixture
def leaf(XR: XerxesRoot):
    leaf = Leaf(Addr(0), XR)

    # check if leaf is connected to the network first
    ping = leaf.ping()
    assert ping is not None
    assert ping.v_maj == PROTOCOL_VERSION_MAJOR
    assert ping.v_min == PROTOCOL_VERSION_MINOR
    return leaf 


# stress test for reading registers 0-1024
def test_read_stress(XR: XerxesRoot):
    """Reads 1024 registers and prints them in fixed width format

    Args:
        XR (XerxesRoot): XerxesRoot instance
    """

    l = Leaf(Addr(0), XR)
    print("")
    # read 1024 registers in chunk of 4 bytes, 1024/4 = 256
    for i in range(0, 256):
        # read 4 bytes from register i
        xm = l.read_reg(i*4, 4)
        # check if message id is correct
        assert xm.message_id == MsgId.READ_REPLY
        assert len(xm.payload) == 4

        # unpack float
        vf = struct.unpack("f", xm.payload)[0]

        # alteratively unpack unsigned int
        vi = struct.unpack("I", xm.payload)[0]

        # print both in fixed width
        print(f"Reg# {i*4:3d}: {vf:10.3f} {vi:10d}")


def test_read_speed(XR: XerxesRoot):
    l = Leaf(Addr(0), XR)

    start = time.perf_counter_ns()

    chunk = 196
    # read 1024 registers in chunk of 128 bytes, 1024/128 = 8
    for i in range(0, int(1024/chunk)):
        xm = l.read_reg(i*chunk, chunk)
        assert xm.message_id == MsgId.READ_REPLY
        assert len(xm.payload) == chunk

    end = time.perf_counter_ns()

    # calculate speed in bytes per second, 256 registers * 4 bytes per register = 1024 bytes, 1024 bytes / time in seconds
    speed = 1024 / ((end-start)/1e9) # bytes per second
    
    # print speed in kiB/s
    print(f"Read speed: {speed/1024:.2f} kiB/s")

    assert speed > 1024 # 1 kiB/s


def test_write_stress_cycle_time(XR: XerxesRoot):
    """Writes 10 random cycle times to register 32"""
    l = Leaf(Addr(0), XR)
    
    # write 10 random intervals from 100ms to 500ms to register 32
    for i in range(0, 25):
        # create a random interval in microseconds
        cycle_time = random.randint(100000, 500000)
        # create a 4 byte duration value
        cycle_time_bytes = struct.pack("I", cycle_time)
        # send a message to the leaf, which sets the desired cycle time
        rply: XerxesMessage = l.write_reg(32, cycle_time_bytes)

        # check if message id is correct
        assert rply.message_id == MsgId.ACK_OK
        # check if payload is empty
        assert rply.payload == b""
        # check if the cycle time was set correctly
        assert l.read_reg(32, 4).payload == cycle_time_bytes

        # wait for 100ms
        time.sleep(.2)


def test_read_mean_burst(XR: XerxesRoot, leaf: Leaf):
    """Reads 1000 registers in a burst and calculates the mean value of the first 4 bytes

    Args:
        XR (XerxesRoot): XerxesRoot instance
        leaf (Leaf): Leaf instance
    """

    burst = 100
    # read 4 floats from register 272 in burst mode 
    mean = [0, 0, 0, 0]
    for i in range(0, burst):
        xm = leaf.read_reg(272, 16)
        assert xm.message_id == MsgId.READ_REPLY
        vals = struct.unpack("ffff", xm.payload)[:4]

        # add each value to the mean
        for i in range(0, 4):
            mean[i] += vals[i]
    
    # calculate mean
    for i in range(0, 4):
        mean[i] = mean[i] / burst
    
    # print mean values
    print(f"Mean values: {mean[0]:.3f} {mean[1]:.3f} {mean[2]:.3f} {mean[3]:.3f}")
    
    # check if at least one value is not zero
    