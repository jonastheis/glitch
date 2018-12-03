# Week3

# Example Bit-flips

Our code works the best on `node004` and almost always find ~5 flips immediately. See below for an example found in `node005`:

```
[Allocator] + Exhausting memory with 128 page-textures
[Allocator] + Number of textures already created 69 | max_id so far 0
[Allocator] + Created 128 textures and found 197 in the file. Approximate size 0 MB
++ Prepare row [0]
++++ BIT FLIP IDENTIFIED
+++ INFO: [Texture-Id: 132][Bit-Index: 4][Byte-Index: 1021][ByteValue: ef][8-byte-offset: 127]
+++ Bit-value: 11101111
+ KGSLEntry[29+0=29] = {id: 156, texture_id: 132, useraddr: b5e66000, pfn: 47b2c, alloc_order: 0}

  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,
  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,
  ...
  ...
  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ef,ff,ff,
  ...
  ...
  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,
  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,  ff,ff,ff,ff,
```

Live video of the above log is attached. 

For the rest, there are the KGSLEntries (a data structure we use to store the combined data of each KGSL and pagemap entry) of some random bit-flips that we have observed. Technically, the `useraddr` and the `pfn` can be used to obtain the physical address and later verified. Logs were recorded earlier so they have slightly simpler format.

```
+++ [TextId: 2597] BIT FLIP IDENTIFIED [ByteIdx: 2583][ByteValue: fb]!
11111011
+ KGSLEntry[28+0=28] = {id: 2626, texture_id: 2597, useraddr: b3e68000, pfn: 61add, alloc_order: 291}

+++ [TextId: 2615] BIT FLIP IDENTIFIED [ByteIdx: 341][ByteValue: bf]!
10111111
+ KGSLEntry[46+0=46] = {id: 2644, texture_id: 2615, useraddr: b3e44000, pfn: 61aef, alloc_order: 273}
```

# How to Run 

Same as week1 and week2. Use the all in one `prun-it.sh` script with compiled files transferred to DAS.