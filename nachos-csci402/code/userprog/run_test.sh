#!/bin/bash

for i in $(seq 0 $1);
do
	echo "$i"
	nachos -x ../test/multiprogrammed_museum -rs $i
done
