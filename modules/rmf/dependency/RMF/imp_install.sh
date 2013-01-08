#!/bin/sh

mkdir -p $2/lib
ln -sfF $1/libRMF* $1/_RMF.so $1/RMF.py $2/lib/

mkdir -p $2/include/RMF
ln -sfF $3/include/RMF/* $2/include/RMF/
ln -sfF $1/include/RMF/* $2/include/RMF/
ln -sfF $1/include/RMF.h $2/include/
ln -sfF $3/swig/*.i $2/swig

if ! -e $1/AvroCpp exit 0
ln -sfF $1/AvroCpp/libavro* $2/lib/
ln -sfF $3/AvroCpp/avro $2/include
ln -sfF $3/AvroCpp/avrogencpp $2/bin
