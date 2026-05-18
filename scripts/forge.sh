#!/bin/bash
# crush
./build/bin/crush/crush_ppac $1
./build/bin/crush/crush_beam $1
./build/bin/crush/crush_trigger $1
./build/bin/crush/crush_t0d1 $1
./build/bin/crush/crush_t0d2 $1
./build/bin/crush/crush_t0d3 $1
./build/bin/crush/crush_t0d4 $1
./build/bin/crush/crush_others $1
# coke trigger
./build/bin/coke -r $1
./build/bin/crush_vme -v $1
./build/bin/smelt/sift -x $1 -v $1 -t t1
./build/bin/smelt/smelt -r $1 -v $1 -t t1 t0d1 t0d2 t0d3 t0d4 t0s t0csi ppac


 
