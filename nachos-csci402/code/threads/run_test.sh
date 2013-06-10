#!/bin/bash

for i in $(seq 0 9999);
do
	nachos -rs $i
	echo "$i"
done
