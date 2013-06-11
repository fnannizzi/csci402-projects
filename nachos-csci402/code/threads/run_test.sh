#!/bin/bash

for i in $(seq 0 9);
do
	echo "$i"
	nachos -P2 3 -rs $i
done
