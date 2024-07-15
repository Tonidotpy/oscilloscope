# Read and plot waves.h

import matplotlib.pyplot as plt

cnt = 2

f = open("waves.h", "r").read()
f = f[f.find('{'):f.rfind('}')]
functions = f.split('}')[:-1]

for fun in functions:
    name = fun.split('//')[1].split('\n')[0]
    values = fun[fun.find('\t'):]
    values = values.replace(' ', '')
    values = values.replace('\n', '')
    values = values.replace('\t', '')
    values = values.split(',')
    xs = [i for i in range(cnt*len(values))]
    ys = []
    for _ in range(cnt):
        for i in values:
            ys.append(int(i, 16))
    plt.plot(xs, ys, label=name)
plt.legend()
plt.show()