#!/bin/bash

for i in $(seq 0 9999);
do
	echo "$i"
	nachos -P2 -rs $i
done
