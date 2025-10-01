import numpy as np
import pandas as pd

from matplotlib import pyplot as plt

import os
import sys
from scipy.signal import savgol_filter

# Load the data
file = sys.argv[1]
data = pd.read_csv(file)

# big chart format
plt.figure(figsize=(15, 10))


plt.subplot(2, 2, 1)

plt.plot(data["Time"] / 1000, data["Roll"], label="Roll")
plt.plot(data["Time"] / 1000, data["Pitch"], label="Pitch")
plt.plot(data["Time"] / 1000, data["Yaw"], label="Yaw")

plt.legend()

plt.subplot(2, 2, 2)
#smooth data
data["ax"] = savgol_filter(data["ax"], window_length=1, polyorder=0)
data["ay"] = savgol_filter(data["ay"], window_length=1, polyorder=0)
data["az"] = savgol_filter(data["az"], window_length=1, polyorder=0)

plt.plot(data["Time"] / 1000, data["ax"], label="ax")
plt.plot(data["Time"] / 1000, data["ay"], label="ay")
plt.plot(data["Time"] / 1000, data["az"], label="az")

plt.legend()


plt.subplot(2, 2, 3)

for i in range(5):
    plt.plot(data["Time"], data["Channel_" + str(i)], label="Channel_" + str(i))

plt.legend()

plt.subplot(2, 2, 4)

northing = data["Latitude"] - data["Latitude"][0]
easting  = data["Longitude"] - data["Longitude"][0]

northing = northing * 40_075 * 1000 / 360
easting  = easting  * 40_075 * 1000 / 360 * np.cos(data["Latitude"][0] * np.pi / 180)
plt.scatter(easting, northing, s = 1)
plt.xlabel("Easting [m]")
plt.ylabel("Northing [m]")
plt.axis("equal")
plt.show()


