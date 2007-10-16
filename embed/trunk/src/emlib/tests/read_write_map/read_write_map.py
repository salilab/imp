import sys

in_filename = "in.em"
out_filename = "out.em"

from EM import DensityMap
from EM import EMReaderWriter

scene = DensityMap()
xrw = EMReaderWriter()
scene.Read(in_filename,xrw)
scene.Write(out_filename,xrw);
