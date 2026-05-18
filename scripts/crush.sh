#!/bin/bash
./build/bin/crush/crush_ppac $1
./build/bin/crush/crush_beam $1
./build/bin/crush/crush_trigger $1
./build/bin/crush/crush_t0d1 $1
./build/bin/crush/crush_t0d2 $1
./build/bin/crush/crush_t0d3 $1
./build/bin/crush/crush_t0d4 $1
./build/bin/crush/crush_others $1


 
