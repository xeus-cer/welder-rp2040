from xerxes_protocol import (
    XerxesNetwork,
    XerxesRoot,
    Leaf
)

from serial import Serial
import logging
import time

log = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

XN = XerxesNetwork(Serial(port="/dev/ttyUSB0", baudrate=115200, timeout=0.02))
XN.init()
XR = XerxesRoot(0xFE, XN)

for i in range(32):
    try:
        l = Leaf(i, XR)
        l.ping()
        break
    except TimeoutError:
        continue
log.info(f"Found leaf at address {l.address}")
log.info(f"UUID: {l.device_uid}")
    
mean_vals = {}
while True:
    try:
        mean_vals["pv0"] = l.mean_pv0 * 25
        mean_vals["pv1"] = l.mean_pv1 * 25
        
        print(
            f"Measurement:\t{mean_vals['pv0']:.4f}mm"
            f"\tReference:\t{mean_vals['pv1']:.4f}mm"
            f"\tStdDev:\t{(l.std_dev_pv0+l.std_dev_pv1) * 25e3 / 2:.2f}um   ",
            end="\r"
        )
        time.sleep(.04)
    except KeyboardInterrupt:
        break
    except TimeoutError:
        log.warning(f"TimeoutError reading leaf {l.address}")
    