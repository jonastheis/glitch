# Binary analysis

- Example of a corrupted number that stayed a number 
- 0 10000011101 0000000100000101000001010000000000000000000000000000 (original)
- 0 10000011101 0000000100000001000001010000000000000000000000000000 (new) (1077952832)

  - 01000001110100000001000001010000 01010000000000000000000000000000 (original)
  - 01000001110100000001000000010000 01010000000000000000000000000000 (new) (1077952832)

  - 41D01050 50000000 (original)
  - 41D01010 50000000 (new) (1077952832)


- Example of a corrupted numbet that became a NAN 
- 0 10000011101 0000000100000101000001010000000000000000000000000000 (original)
- 0 11111111111 1111111111111000000101000000010000010100000101000000 (new)
- 1 11111111111 1111111111011000000101000000010000010100000101000000 (new)
- 1 11111111111 0111111111111000000101000000010000010100000101000000 (new)
- Example of a corrupted numbet that became a 64Double
- 01000001 11010000 00010000 01010000 01010000 00000000 00000000 00000000 (original)
- 11111111 10111111 11111111 10000001 01000000 01000001 01000001 01000000 (new) (-2.2469806059288464e+307)
- 11111111 11011111 11111111 10000001 01000000 01000001 01000001 01000000 (new) (-8.987922423715386e+307)
- 11110111 11111111 11111111 10000001 01000000 01000001 01000001 01000000 (new) (-1.0565252034706398e+270)


- A good run on node 5
 [Exploit] +++ Array value change detected: targetPool[1732][403] =>  NaN exploit.js:87:9
 new0	0 00000000001 1110111111111000000101000000010000010100000101000000 exploit.js:88:9
 new1	1 11111111111 1110111111111000000101000000010000010100000101000000 exploit.js:89:9
 old	0 10000011101 0000000100000101000001010000000000000000000000000000 exploit.js:90:9
 [Exploit] +++ Array value change detected: targetPool[2573][427] =>  -8.987922423715386e+307 exploit.js:87:9
 new0	1 11111111110 0000000000000111111010111111101111101011111011000000 exploit.js:88:9
 new1	1 11111111101 1111111111111000000101000000010000010100000101000000 exploit.js:89:9
 old	0 10000011101 0000000100000101000001010000000000000000000000000000 exploit.js:90:9
 [Exploit] +++ Array value change detected: targetPool[6464][171] =>  NaN exploit.js:87:9
 new0	0000000000011111111111011000000101000000010000010100000101000000 exploit.js:88:9
 new1	11111111 11111111 11111101 10000001 01000000 01000001 01000001 01000000 exploit.js:89:9
 old	01000001 11010000 00010000 01010000 01010000 00000000 00000000 00000000 exploit.js:90:9
 [Exploit] +++ Array value change detected: targetPool[6921][351] =>  -5.4857924949434726e+303 exploit.js:87:9
 new0	1111111111100000000000000111111010111111101111101011111011000000 exploit.js:88:9
 new1	11111110 11111111 11111111 10000001 01000000 01000001 01000001 01000000 exploit.js:89:9
 old	01000001 11010000 00010000 01010000 01010000 00000000 00000000 00000000 exploit.js:90:9
 [Exploit] +++ Array value change detected: targetPool[7338][20] =>  NaN exploit.js:87:9
 new0	0000000000011111111101111000000101000000010000010100000101000000 exploit.js:88:9
 new1	11111111 11111111 11110111 10000001 01000000 01000001 01000001 01000000 exploit.js:89:9
 old	01000001 11010000 00010000 01010000 01010000 00000000 00000000 00000000 exploit.js:90:9
 [Exploit] +++ Array value change detected: targetPool[7594][484] =>  NaN exploit.js:87:9
 new0	0000000000011110111111111000000101000000010000010100000101000000 exploit.js:88:9
 new1	11111111 11111110 11111111 10000001 01000000 01000001 01000001 01000000 exploit.js:89:9
 old	01000001 11010000 00010000 01010000 01010000 00000000 00000000 00000000 expl