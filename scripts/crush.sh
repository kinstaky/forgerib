#!/bin/bash

read -p "run number: " num

./build/bin/crush/crush_ppac $num
./build/bin/crush/crush_beam $num
./build/bin/crush/crush_trigger $num
./build/bin/crush/crush_t0d1 $num
./build/bin/crush/crush_t0d2 $num
./build/bin/crush/crush_t0d3 $num
./build/bin/crush/crush_t0d4 $num
./build/bin/crush/crush_others $num


 
