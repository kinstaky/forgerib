#!/bin/bash

read -p "run number: " num

./build/bin/decode/decode_ppac $num
./build/bin/decode/decode_beam $num
./build/bin/decode/decode_trigger $num
./build/bin/decode/decode_t0d1 $num
./build/bin/decode/decode_t0d2 $num
./build/bin/decode/decode_t0d3 $num
./build/bin/decode/decode_t0d4 $num
./build/bin/decode/decode_others $num


 
