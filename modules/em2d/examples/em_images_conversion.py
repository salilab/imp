## \example em2d/em_images_conversion.py
# Conversion of Electron Microscopy Images.
#

import IMP
import IMP.em2d
import sys

IMP.setup_from_argv(sys.argv, "EM images conversion")

"""
    Conversion of Electron Microscopy Images.

"""

# Read images
fn_selection = IMP.em2d.get_example_path("all-1z5s-projections.sel")

srw = IMP.em2d.SpiderImageReaderWriter()
trw = IMP.em2d.TIFFImageReaderWriter()
fn_images = IMP.em2d.read_selection_file(fn_selection)
fn_images = [IMP.em2d.get_example_path(x) for x in fn_images]
images = IMP.em2d.read_images(fn_images, srw)
# write
fn_saved = IMP.em2d.create_filenames(3, "1z5s-projection", "tif")
IMP.em2d.save_images(images, fn_saved, trw)
