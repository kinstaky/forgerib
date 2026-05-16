#!/bin/bash
./build/bin/forge/forge -r $1 -t main trigger
./build/bin/forge/map_vme -v $(($1-2))
./build/bin/forge/align -x $1 -v $(($1-2)) -t t1
./build/bin/forge/forge -r $1 -t t1 trigger t0d1 t0d2 t0d3 t0d4 t0s t0csi
./build/bin/fuse -x $1 -v $(($1-2)) -t t1 t0csi
