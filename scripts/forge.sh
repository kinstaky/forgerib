#!/bin/bash

read -p "run number: " num

./build/bin/forge/forge -r $num  -t main trigger beam ppac t0d1 t0d2 t0d3 t0d4 t0s csi 

