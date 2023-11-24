#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from xerxes_protocol import (
    Leaf,
    LeafConfig
)
import time
import logging
import pytest
_log = logging.getLogger(__name__)


__author__ = "theMladyPan"
__date__ = "2023-07-25"
 
def test_read_mean_val(findLeaf: Leaf):
    """Reads 4 mean values and checks if any of them are not 0"""

    _log.info("Mean PV0: %s", findLeaf.mean_pv0)
    _log.info("Mean PV1: %s", findLeaf.mean_pv1)
    _log.info("Mean PV2: %s", findLeaf.mean_pv2)
    _log.info("Mean PV3: %s", findLeaf.mean_pv3)
    
    # assert that at least one value is not 0
    assert any([
        findLeaf.mean_pv0,
        findLeaf.mean_pv1,
        findLeaf.mean_pv2,
        findLeaf.mean_pv3
    ])
    
def test_stddev(findLeaf: Leaf):
    stdev = [
        findLeaf.std_dev_pv0,
        findLeaf.std_dev_pv1,
        findLeaf.std_dev_pv2,
        findLeaf.std_dev_pv3
    ]

    for i in range(len(stdev)):
        #_log.info(f"Stddev ch[{i}]: {stdev[i]}, um: {stdev[i] * 25e3}")
        # format for 3 decimal places, use math notation for um
        _log.info(f"Stddev ch[{i}]: {stdev[i]:.3e}, um: {stdev[i] * 25e3:.2f}")

    assert all(x < 1e-4 for x in stdev)