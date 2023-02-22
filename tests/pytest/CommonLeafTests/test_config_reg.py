from xerxes_protocol import (
    Leaf,
    XerxesRoot,
    LeafConfig
)
import time


__author__ = "theMladyPan"
__date__ = "2023-02-22"


def test_read_write_config(cleanLeaf: Leaf, XR: XerxesRoot):
    """ Writes 1 byte to register 40 (config) to set the leaf to free run mode and calculate statistics

    Args:
        XR (XerxesRoot): The root of the network
    """
    leaf = cleanLeaf


    # make config byte
    config_bits = LeafConfig.freeRun | LeafConfig.calcStat

    leaf.device_config = config_bits

    # wait for the leaf to acquire some samples
    time.sleep(0.1)

    # assert that the config bits are set correctly
    assert leaf.device_config == config_bits
    assert leaf.device_config & LeafConfig.freeRun == LeafConfig.freeRun
    assert leaf.device_config & LeafConfig.calcStat == LeafConfig.calcStat

    # assert that process data is available
    assert leaf.pv0 != 0
    assert leaf.std_dev_pv0 != 0
    assert leaf.min_pv0 != 0
    assert leaf.max_pv0 != 0
