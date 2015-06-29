#!/usr/bin/env python

import avro.schema
from avro.datafile import DataFileReader, DataFileWriter
from avro.io import DatumReader, DatumWriter
import sys

reader = DataFileReader(open(sys.argv[1], "r"), DatumReader())
for user in reader:
    print user
reader.close()
