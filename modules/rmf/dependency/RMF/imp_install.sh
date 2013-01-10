#!/bin/sh

mkdir -p $2/lib
if [ -e $1/_RMF.pyd ]; then
  # Windows-style library names
  ln -sfF $1/RMF.* $1/_RMF.* $2/lib/
  ln -sfF $1/RMF.lib $2/lib/libRMF.lib
else
  # Unix-style library names
ln -sfF $1/libRMF* $1/_RMF.so $1/RMF.py $2/lib/
fi

mkdir -p $2/include/RMF
ln -sfF $3/include/RMF/* $2/include/RMF/
ln -sfF $1/include/RMF/* $2/include/RMF/
ln -sfF $1/include/RMF.h $2/include/
mkdir -p $2/swig
ln -sfF $3/swig/*.i $2/swig

if [ -e $1/AvroCpp ]; then
  ln -sfF $1/AvroCpp/libavro* $2/lib/
  ln -sfF $3/AvroCpp/avro $2/include
  mkdir -p $2/bin
  ln -sfF $1/AvroCpp/avrogencpp $2/bin
fi
