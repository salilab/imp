
import IMP
import IMP.em2d as em2d
import os
from math import *
import re
import sys

if(len(sys.argv) <2):
    print "Parameters: <1>"
    print "<1> Selection file with the names of the the EM images"
    sys.exit()

fn_images_names = sys.argv[1]
images_names = em2d.read_selection_file(fn_images_names)
srw = em2d.SpiderImageReaderWriter()
images=em2d.read_images(images_names,srw)
# Replace the extension .spi for .jpg
jpg_names=[]
for s in images_names:
    jpg_names.append(re.sub(".spi",".jpg",s))
jrw = em2d.JPGImageReaderWriter()
em2d.save_images(images,jpg_names,jrw)
