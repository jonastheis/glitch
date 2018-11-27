import numpy as np
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

# Good ref: https://matplotlib.org/tutorials/introductory/usage.html#sphx-glr-tutorials-introductory-usage-py

RANDOM_MARKER = "o"
GUIDED_MARKER = "^"
Y_TICKS = [
0, 
128,
256,
384,
512,
640,
768,
896,
1024,
1152,
1280,
]

REQUESTS = [
0, 
128,
256,
384,
512,
640,
768,
896,
1024,

1056,
1088,
1120,

1152,
1280,
]

misses = map(lambda x: x/16,[
0,
1024,
2048,
3072,
4096,
5120,
6144,
7168,
8192,

10752,
13312,
15872,

18432,
20480,
])

linear = np.linspace(0, 1152, 1152)

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

plt.title("UCHE Cache")

plt.legend()

plt.show()