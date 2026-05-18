#!/bin/bash
./build/bin/coke -r $1
./build/bin/crush_vme -v $(($1-2))
./build/bin/smelt/sift -x $1 -v $(($1-2)) -t t1
./build/bin/smelt/smelt -r $1 -v $(($1-2)) -t t1 t0d1 t0d2 t0d3 t0d4 t0s t0csi
