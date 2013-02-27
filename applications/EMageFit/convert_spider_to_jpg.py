#!/usr/bin/env python

import IMP
import IMP.em2d as em2d
import os
from math import *
import re

parser = IMP.OptionParser(usage="""%prog [options] selfile

selfile is the selection file with the names of the the EM images""",
                          imp_module=em2d)
opts, args = parser.parse_args()
if len(args) != 1:
    parser.error("wrong number of arguments")

fn_images_names = args[0]
images_names = em2d.read_selection_file(fn_images_names)
srw = em2d.SpiderImageReaderWriter()
images=em2d.read_images(images_names,srw)
# Replace the extension .spi for .jpg
jpg_names=[]
for s in images_names:
    jpg_names.append(re.sub(".spi",".jpg",s))
jrw = em2d.JPGImageReaderWriter()
em2d.save_images(images,jpg_names,jrw)
