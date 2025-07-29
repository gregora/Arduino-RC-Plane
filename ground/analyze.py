import numpy as np
import pandas as pd

from matplotlib import pyplot as plt

import os
import sys

# Load the data
file = sys.argv[1]
data = pd.read_csv(file)

# Plot roll, pitch, and yaw on one graph
plt.subplot(2, 1, 1)

#plt.plot(data["Time"], data["Roll"], label="Roll")
#plt.plot(data["Time"], data["Pitch"], label="Pitch")
#plt.plot(data["Time"], data["Yaw"], label="Yaw")

plt.plot(data["Time"], data["ax"], label="ax")
plt.plot(data["Time"], data["ay"], label="ay")
plt.plot(data["Time"], data["az"], label="az")

plt.legend()


# Plot channels on another graph
plt.subplot(2, 1, 2)

for i in range(5):
    plt.plot(data["Time"], data["Channel_" + str(i)], label="Channel_" + str(i))

plt.legend()

plt.show()


