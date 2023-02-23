from xerxes_protocol import (
    Leaf,
    LeafConfig,
    ERROR_MASK_SENSOR_OVERLOAD
)

import time
import logging
_log = logging.getLogger(__name__)


__author__ = "theMladyPan"
__date__ = "2023-02-23"


def test_low_cycle_time_overload(cleanLeaf: Leaf):
    """Test sensor overload error flag.
    
    Test whether setting up low device cycle time causes overload on the sensor causing it to set the overload flag.
    
    Args:
        cleanLeaf (Leaf): The leaf to test
    """

    # put sensor to free run mode and enable statistics calculation 
    cleanLeaf.device_config |= LeafConfig.freeRun | LeafConfig.calcStat

    # check that the leaf is in free run mode
    assert (
        cleanLeaf.device_config & LeafConfig.freeRun | LeafConfig.calcStat == LeafConfig.freeRun | LeafConfig.calcStat
    ), "Leaf is not in free run mode"

    # wait for sensor to update
    time.sleep(1)

    # read cycle time
    cycle_time_us = cleanLeaf.net_cycle_time_us
    _log.debug(f"Cycle time is {cycle_time_us} us")

    assert cycle_time_us > 0, "Cycle time is 0 - device is not in free run mode"

    # set desired cycle time to 1ms less than current cycle time 
    # (to be sure that it is lower than sensor cycle time) but still positive value
    desired_cycle_time_us = max(0, cycle_time_us - 1000)
    _log.debug(f"Setting cycle time to {desired_cycle_time_us} us")

    # set cycle time
    cleanLeaf.desired_cycle_time_us = desired_cycle_time_us

    # wait for sensor to update
    time.sleep(.1)

    # by now the sensor should have set the overload flag
    errors = cleanLeaf.device_error
    _log.debug(f"Errors: {errors}")

    assert errors & ERROR_MASK_SENSOR_OVERLOAD, "Sensor overload flag was not set"
