# Week3

## Hammer Pattern
To visualise and make describing our hammer pattern a bit easier we will refer to the following representation of order 6 contiguous memory. It displays the hammering of row 1 in bank 0. Depending on which row we are hammering we choose the textures for idle accesses from the row which is not used. To save some time and prevent unnecessary row misses we choose this 5 textures either from the end or beginning of this row, again it depends on the bank which is currently hammered.

```
Example for hammering first bank --=unused 4KB texture, xx=hammer texture, ..=eviction texture, ##=victim
 0001 0203 0405 0607 0809 1011 1213 1415
|xxxx|----|----|----|----|----|----|----|
 1617 1819 2021 2223 2425 2627 2829 3031
|####|----|----|----|----|----|----|----|
 3233 3435 3637 3839 4041 4243 4445 4647
|xxxx|----|----|----|----|----|----|----|
 4849 5051 5253 5455 5657 5859 6061 6263
|----|----|----|----|----|--..|....|....|
```

### UCHE
We choose to alternate between idle and hammering accesses to minimise the time between two hammering accesses. Also the accesses to `00` and `01` are not immediately after each other to prevent the DRAM from reading `01` from the bank's rowbuffer but cause a row miss. Access `8` already evicts the access from `0` from the cache. Thus, repeating this pattern constantly creates cache misses for every request due to the FIFO replacement policy of the cache. 

```
access | texture | KB offset | type
     0 |      00 |         0 | hammer
     1 |      59 |       236 | idle
     2 |      32 |       128 | hammer
     3 |      60 |       240 | idle
     4 |      01 |         4 | hammer
     5 |      61 |       244 | idle
     6 |      33 |       132 | hammer
     7 |      62 |       248 | idle
     8 |      63 |       252 | idle
```

### L1
The L1 cache has 16 ways of which 8 ways get already filled with the accesses to UCHE. By accessing every texture/page with a 32 byte offset, starting with `00 + 32 Byte`, we simultaneously load that texture/page from memory (hammer) and fill up a cacheline in L1. Access `8` on `63.32` already evicts the access from `0` (from accessing UCHE before) from the cache. Thus, repeating this pattern constantly creates cache misses for every request due to the FIFO replacement policy of the cache. 

```
access | texture | KB offset | type
     0 |   00.32 |      0.32 | hammer
     1 |   59.32 |     36.32 | idle
     2 |   32.32 |    128.32 | hammer
     3 |   60.32 |    240.32 | idle
     4 |   01.32 |      4.32 | hammer
     5 |   61.32 |    244.32 | idle
     6 |   33.32 |    132.32 | hammer
     7 |   62.32 |    248.32 | idle
     8 |   63.32 |    252.32 | idle
```