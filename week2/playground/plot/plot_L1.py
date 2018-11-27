import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

# Good ref: https://matplotlib.org/tutorials/introductory/usage.html#sphx-glr-tutorials-introductory-usage-py

RANDOM_MARKER = "o"
GUIDED_MARKER = "^"
Y_TICKS = [
0, 
32,
64, 
96, 
128, 
160, 
192,
]

REQUESTS = [
0, 
32,
64, 
96, 
128, 
130,
132,
134,
136,
138,
140,
160, 
192,
]

misses = [
0,
16,
32,
48,
64,
82,
100,
102,
104,
122,
140,
160, 
192,
]

linear = np.linspace(0, 160, 160)

plt.plot(linear, linear, label='miss == req')
plt.plot(REQUESTS, misses, label="measured values", marker=RANDOM_MARKER)

# set ticks 
plt.yticks(Y_TICKS)
plt.xticks(REQUESTS)
plt.minorticks_on()

# enable grid 
plt.grid()

plt.xlabel('Requests')
plt.ylabel('Misses')

plt.title("L1 Cache")

plt.legend()

plt.show()