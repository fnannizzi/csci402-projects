#!/bin/bash

file_original=test.txt
file_copy=test_output.txt
str1="^@"
str2="^@^"
str3="^@^@"
str4=""
# Copy the file over
cp -i $file_original $file_copy

# Begin conversion

# Remove garbage
sed 's/$str1/$str4/g' $file_copy >$file_copy.txt
mv $file_copy.txt $file_copy

# Remove garbage
sed 's/$str2/$str4/' $file_copy >$file_copy.txt
mv $file_copy.txt $file_copy

# Remove garbage
sed 's/$str3/$str4/' $file_copy >$file_copy.txt
mv $file_copy.txt $file_copy

sed -e 's/^/						/' $file_copy >$file_copy.txt
mv $file_copy.txt $file_copy
