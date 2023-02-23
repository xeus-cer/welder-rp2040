from xerxes_protocol import (
    Addr,
    Leaf,
    XerxesPingReply,
    XerxesRoot
)

import pytest


__author__ = "theMladyPan"
__date__ = "2023-02-22"


def test_ping_one(XR: XerxesRoot):
    leaf = Leaf(Addr(0), XR)
    rply: XerxesPingReply = leaf.ping() 

    print(rply)
    
    assert XR.isPingLatest(rply)
    assert rply.latency > 0 and rply.latency < 0.1  # should be less than 100ms


# find all leaves on the network
@pytest.mark.skip("This test takes a long time to run")
def test_ping_all(XR: XerxesRoot):
    found = []
    for dev_addr in range(0, 256):
        leaf = Leaf(Addr(dev_addr), XR)
        try:
            rply: XerxesPingReply = leaf.ping() 

            if rply is not None:
                found.append(leaf)
                print(rply)
                assert XR.isPingLatest(rply)

        except TimeoutError:
            pass
    
    # print all found leaves
    for leaf in found:
        print(leaf)