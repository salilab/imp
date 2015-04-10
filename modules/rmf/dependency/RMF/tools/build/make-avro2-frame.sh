#!/bin/bash

export LANG=C
avrogencpp -p avrocpp/api -n rmf_raw_avro2 -i src/backend/avro2/Frame.json -o src/backend/avro2/raw_frame.h
sed -i.old "s#namespace avro#namespace internal_avro#g" src/backend/avro2/raw_frame.h
sed -i.old "s#avro::#internal_avro::#g" src/backend/avro2/raw_frame.h