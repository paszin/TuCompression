# TuCompression


## Huffman

* Result of Huffman Encoding is: 
  * one dictionary `code : original value`
  * one attribute vector with N-bit-bitsets. `code_code_code, code_code, code_code_code`
  * one attribute vector with pairs of lower bound and upper bound per chunk `(min1, max1); (min2, max2)`

* Design Decisions:
  * bounds vector introduces overhead in size, but improves performance for selection and aggregation. 
    * How? check if a value is in between the bounds, if not: do not decompress the chunk
