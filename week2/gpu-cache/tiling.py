TH = 32.0
TW = 32.0

W = 8
H = 8

TILE = 4


def f(x, y):
	return 	( ( (y/TH) * ((W+TW-1)/TW) + (x/TW) ) * (TW*TH) ) + \
			( (y%TH) * (TW) ) + \
			( x % TW )


def offToPix(t):
	tile_number = int(t / (TILE*TILE))
	tile_row = int(tile_number / ((TW/TILE))) * TILE
	tile_col = int(tile_number % (TW/TILE)) * TILE

	# internal offset 
	local_x = t % TILE
	# Has to be internal within the tile. Draw it and you will get it.
	local_y = int((t-(tile_number*TILE*TILE)) / TILE)

	# print('number', tile_number, 'trow', tile_row, 'tcol', tile_col, 'local_x', local_x, 'local_y', local_y)

	global_x = local_x + tile_col 
	global_y = local_y + tile_row 

	return (global_x, global_y)


# print(offToPix(137))
for i in range(1024): 
	print(i, offToPix(i))

# inverse = {}
# for h in range(int(H)):
# 	for w in range(int(W)):
# 		z = f(w,h)
# 		print("{},{} => {}".format(w, h, z))
# 		if not z in inverse:
# 			inverse[z] = [(w, h)]
# 		else:
# 			inverse[z].append((w, h))


# for key in sorted(inverse.keys()):
# 	if key % 16 == 0:
# 		print("\t{} => {}".format(key, inverse[key]))
	# else:
	# 	print("{} => {}".format(key, inverse[key]))
# print(inverse)