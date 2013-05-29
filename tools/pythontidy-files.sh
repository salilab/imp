#!/usr/bin/env sh

for i in $*; do
echo $i
tools/python/PythonTidy.py $i $i
done
