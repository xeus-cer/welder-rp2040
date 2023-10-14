from xerxes_protocol import (
    XerxesNetwork,
    XerxesRoot,
    Leaf
)

from serial import Serial
import logging
import time
from asciichartpy import plot

log = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

XN = XerxesNetwork(Serial(port="/dev/ttyUSB0", baudrate=115200, timeout=0.1))
XN.init()
XR = XerxesRoot(0xFE, XN)


leaf = Leaf(0, XR)

log.info(f"Found leaf at address {leaf.address}")
log.info(f"UUID: {leaf.device_uid}")
    
mean_vals = {}
pv0_series = []
pv3_series = []

while True:
    try:
        mean_vals["pv0"] = leaf.mean_pv0 * 25
        mean_vals["pv3"] = leaf.mean_pv3
        pv0_series.append(mean_vals["pv0"])
        pv3_series.append(mean_vals["pv3"])
        
        # Limit the length of time series for visualization
        if len(pv0_series) > 100:
            del pv0_series[0]
            del pv3_series[0]

        # print(
        #     f"Measurement:\t{mean_vals['pv0']:.4f}mm"
        #     f"\tReference:\t{mean_vals['pv1']:.4f}mm"
        #     f"\tStdDev:\t{(leaf.std_dev_pv0+leaf.std_dev_pv1) * 25e3 / 2:.2f}um   ",
        #     end="\r"
        # )

        # Plot the time series in terminal
        
        print("PV0 Time Series")
        print(plot(pv0_series, {'height': 20, 'format': '{:.6f}'}, ))
        print("PV3 Time Series")
        print(plot(pv3_series, {'height': 20, 'format': '{:.6f}'}, ))
        
        
        #time.sleep(.1)
        # clear screen
        print("\033c", end="")
    except KeyboardInterrupt:
        del XN
        break
    except TimeoutError:
        log.warning(f"TimeoutError reading leaf {leaf.address}")
        exit(1)
