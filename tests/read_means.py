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


l = Leaf(8, XR)

log.info(f"Found leaf at address {l.address}")
log.info(f"UUID: {l.device_uid}")
    
mean_vals = {}
pv0_series = []
pv1_series = []

while True:
    try:
        mean_vals["pv0"] = l.mean_pv0 * 25
        mean_vals["pv1"] = l.mean_pv1 * 25
        pv0_series.append(mean_vals["pv0"])
        pv1_series.append(mean_vals["pv1"])
        
        # Limit the length of time series for visualization
        if len(pv0_series) > 100:
            del pv0_series[0]
            del pv1_series[0]

        # print(
        #     f"Measurement:\t{mean_vals['pv0']:.4f}mm"
        #     f"\tReference:\t{mean_vals['pv1']:.4f}mm"
        #     f"\tStdDev:\t{(l.std_dev_pv0+l.std_dev_pv1) * 25e3 / 2:.2f}um   ",
        #     end="\r"
        # )

        # Plot the time series in terminal
        
        print("PV0 Time Series")
        print(plot(pv0_series, {'height': 15, 'format': '{:.4f}'}, ))
        print("PV1 Time Series")
        print(plot(pv1_series, {'height': 15, 'format': '{:.4f}'}, ))
        
        time.sleep(.5)
        # clear screen
        print("\033c", end="")
    except KeyboardInterrupt:
        break
    except TimeoutError:
        log.warning(f"TimeoutError reading leaf {l.address}")