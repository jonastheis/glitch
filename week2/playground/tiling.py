TH = 4.0
TW = 4.0

W = 64.0
H = 64.0

def f(x, y):
	return 	( ( (y/TH) * ((W+TW-1)/TW) + (x/TW) ) * (TW*TH) ) + \
			( (y%TH) * (TW) ) + \
			( x % TW )

inverse = {}
for w in range(int(W)):
	for h in range(int(H)):
		z = f(w,h)
		if not z in inverse:
			inverse[z] = [(w, h)]
		else:
			inverse[z].append((w, h))

for key in sorted(inverse.keys()):
	if key % 16 == 0:
		print("\t{} => {}".format(key, inverse[key]))
	# else:
	# 	print("{} => {}".format(key, inverse[key]))
# print(inverse)