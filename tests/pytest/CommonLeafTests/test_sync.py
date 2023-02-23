from xerxes_protocol import (
    Leaf,
    XerxesMemoryMap,
    LeafConfig
)
import struct
import time


__author__ = "theMladyPan"
__date__ = "2023-02-22"


def test_sync_no_stats(cleanLeaf: Leaf):
    """
    Tests the sync method of the leaf

    Args:
        XR (XerxesRoot): The root of the network
        leaf (Leaf): The leaf to test
    """

    cleanLeaf.device_config = LeafConfig.clean    
    # check that the leaf is not in free run mode
    assert cleanLeaf.device_config & LeafConfig.freeRun == 0
    # check that the leaf is not in calc stat mode
    assert cleanLeaf.device_config & LeafConfig.calcStat == 0

    # clean the process value since they are updated during sensor init
    cleanLeaf.pv0 = 0
    cleanLeaf.pv1 = 0
    cleanLeaf.pv2 = 0
    cleanLeaf.pv3 = 0

    # last values should be 0 so do standard deviations
    # read 16 bytes from pv0-pv3
    vals_raw = cleanLeaf.read_reg(XerxesMemoryMap.pv0.elem_addr, 4 * 4)
    std_devs_raw = cleanLeaf.read_reg(XerxesMemoryMap.std_dev_pv0.elem_addr, 4 * 4)
    # convert to floats
    vals = struct.unpack("ffff", vals_raw.payload)[:4]
    std_devs = struct.unpack("ffff", std_devs_raw.payload)[:4]

    # check that all values are 0
    assert not any(vals), "Values were not 0 - device is still in free run mode"
    assert not any(std_devs), "Standard deviations were not 0 - device is still in calc stat mode"
    
    cleanLeaf.root.sync()
    # wait for sensor to update
    time.sleep(.2)

    # read 16 bytes from pv0-pv3
    vals_raw = cleanLeaf.read_reg(XerxesMemoryMap.pv0.elem_addr, 4 * 4)
    std_devs_raw = cleanLeaf.read_reg(XerxesMemoryMap.std_dev_pv0.elem_addr, 4 * 4)
    # convert to floats
    vals = struct.unpack("ffff", vals_raw.payload)[:4]
    std_devs = struct.unpack("ffff", std_devs_raw.payload)[:4]

    # check that at least one value is not 0
    assert any(vals), "No values were updated"
    assert not any(std_devs)
    

def test_sync_with_stats(cleanLeaf: Leaf):

    cleanLeaf.device_config = LeafConfig.calcStat
    # check that the leaf is not in free run mode
    assert cleanLeaf.device_config & LeafConfig.freeRun == 0
    # check that the leaf is in calc stat mode
    assert cleanLeaf.device_config & LeafConfig.calcStat == LeafConfig.calcStat
    
    cleanLeaf.pv0 = 0
    cleanLeaf.pv1 = 0
    cleanLeaf.pv2 = 0
    cleanLeaf.pv3 = 0

    # last values should be 0 so do standard deviations
    # read 16 bytes from pv0-pv3
    vals_raw = cleanLeaf.read_reg(XerxesMemoryMap.pv0.elem_addr, 4 * 4)
    std_devs_raw = cleanLeaf.read_reg(XerxesMemoryMap.std_dev_pv0.elem_addr, 4 * 4)
    # convert to floats
    vals = struct.unpack("ffff", vals_raw.payload)[:4]
    std_devs = struct.unpack("ffff", std_devs_raw.payload)[:4]

    # check that all values are 0
    assert not any(vals), "Values were not 0 - device is still in free run mode"
    assert not any(std_devs), "Standard deviations were not 0 - device is still in calc stat mode"

    # sync twice to gather data for standard deviations
    cleanLeaf.root.sync()
    time.sleep(.1)
    cleanLeaf.root.sync()
    # wait for sensor to update
    time.sleep(.1)

    # read 16 bytes from pv0-pv3
    vals_raw = cleanLeaf.read_reg(XerxesMemoryMap.pv0.elem_addr, 4 * 4)
    std_devs_raw = cleanLeaf.read_reg(XerxesMemoryMap.std_dev_pv0.elem_addr, 4 * 4)
    # convert to floats
    vals = struct.unpack("ffff", vals_raw.payload)[:4]
    std_devs = struct.unpack("ffff", std_devs_raw.payload)[:4]

    # check that at least one value is not 0
    assert any(vals), "No values were updated"
    assert any(std_devs)
