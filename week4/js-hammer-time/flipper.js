'use strict';

const MARKER_VAL = "50515150??FFFFFF40414140??FFFFFF"

/*---------------------------------------------------------

					PYTHON USED FUNCTIONS

----------------------------------------------------------*/






function find_pages(val) {
	if (val==null) {
 		val = "50515150??FFFFFF40414140??FFFFFF";
	}
	console.log(val)
	var ranges = Process.enumerateRangesSync({protection: "rw-", coalesce: false});
	var results = [];
	var arr = null;
	for (var i = 0; i < ranges.length; i++) {
		try {
				arr = Memory.scanSync(ptr(ranges[i].base), ranges[i].size, val);
				if (arr.length > 0) {
					results.push(arr)
					console.log(JSON.stringify(arr))		
				}
			
		} catch (e) {
		}
	}
	// console.log("Location at:", JSON.stringify(location));
	var flatten = []
	for (var i = 0; i < results.length; i++) {
		for (var j = 0; j < results[i].length; j++) {
			flatten.push(results[i][j])
		}
	}

	return flatten;

}




function fill_buff(src, dst, len) {
	for (var i = 0; i < len; i++) {
		dst[i] = src[i];
	}
	return dst
}


function flip_bit(v_addr, bit_to_flip) {

	if (bit_to_flip > 32) {
		v_addr += 4;
		bit_to_flip -= 32;	
	}
	var prev_val = Memory.readPointer(ptr(v_addr));
	var new_val = ptr(prev_val.and(ptr(1<<bit_to_flip).not()));
	Memory.writePointer(ptr(v_addr), new_val);
	console.log("prev_val: " + prev_val + " new_val: " + Memory.readPointer(ptr(v_addr)));
}



rpc.exports = {
  findPages: find_pages,
  flipBit: flip_bit
};


