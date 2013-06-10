#!/bin/bash

for i in $(seq 0 9999);
do
	echo "$i"
	nachos -rs $i
done
