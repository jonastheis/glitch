TH = 4.0
TW = 4.0

W = 128.0
H = 128.0

def f(x, y):
	return 	( ( (y/TH) * ((W+TW-1)/TW) + (x/TW) ) * (TW*TH) ) + \
			( (y%TH) * (TW) ) + \
			( x % TW )

# def f(x, y):
# 	return 	(4 * y * (w+3)) + \
# 			4*x + \
# 			(y%4)*4 + \
# 			x % 4


inverse = {}
for w in range(int(W)):
	for h in range(int(H)):
		z = f(w,h)
		# print("[x={}, y={}] f = {}".format(w, h, z))
		if not z in inverse:
			inverse[z] = [(w, h)]
		else:
			inverse[z].append((w, h))

for key in sorted(inverse.keys()):
	if key % 16 == 0:
		print("{} [{}] => {}".format(key, inverse[key]))
# print(inverse)