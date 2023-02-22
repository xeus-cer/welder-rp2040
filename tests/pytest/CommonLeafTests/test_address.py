#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pytest
from xerxes_protocol import (
    Addr,
    Leaf,
    XerxesRoot
)


__author__ = "theMladyPan"
__date__ = "2023-02-22"


def test_change_address(cleanLeaf: Leaf, XR: XerxesRoot):
    test_addr = 0x55

    # change address, effective immediately
    cleanLeaf.device_address = test_addr
    
    # device should not be reachable at old address
    with pytest.raises(TimeoutError):
        cleanLeaf.ping()

    # create new leaf with new address
    leaf = Leaf(Addr(test_addr), XR)
    
    # test if new address is correct, read address from the device
    new_addr = leaf.device_address

    assert new_addr == test_addr

    # change address back to default, should be effective immediately
    leaf.address = 0

    # device should be reachable at new address because we used "address" property which 
    # handles the address change internally

    assert XR.isPingLatest(leaf.ping())
