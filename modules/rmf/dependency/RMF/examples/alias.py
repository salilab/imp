## \example alias.py
# Show using aliases to provide two different organization schemes
# for manipulating a hierarchy.
from __future__ import print_function
import RMF

# find the name for a temporary file to use to for writing the hdf5 file
tfn = RMF._get_temporary_file_path("aliases.rmf")
print("File is", tfn)

# open the temporary file, clearing any existing contents
fh = RMF.create_rmf_file(tfn)

# add a frame to the file
fh.add_frame("root", RMF.FRAME)

rh = fh.get_root_node()
# add a bunch of "molecules" with colors
mrh = rh.add_child("molecules", RMF.REPRESENTATION)

pf = RMF.ParticleFactory(fh)
cf = RMF.ColoredFactory(fh)

red = []
green = []
blue = []
for i in range(0, 10):
    curph = mrh.add_child("mol" + str(i), RMF.REPRESENTATION)
    for j in range(0, 10):
        curh = curph.add_child("atom" + str(j), RMF.REPRESENTATION)
        p = pf.get(curh)
        p.set_coordinates(RMF.Vector3(3 * i, 3 * j, 0))
        p.set_mass(1)
        p.set_radius(1)
        c = cf.get(curh)
        if j % 3 == 0:
            c.set_rgb_color(RMF.Vector3(1, 0, 0))
            red.append(curh)
        elif j % 3 == 1:
            c.set_rgb_color(RMF.Vector3(0, 1, 0))
            green.append(curh)
        else:
            c.set_rgb_color(RMF.Vector3(0, 0, 1))
            blue.append(curh)

# add a second organization scheme by color
crh = rh.add_child("colors", RMF.REPRESENTATION)

redh = crh.add_child("red", RMF.REPRESENTATION)
greenh = crh.add_child("green", RMF.REPRESENTATION)
blueh = crh.add_child("blue", RMF.REPRESENTATION)

af = RMF.AliasFactory(fh)
for r in red:
    af.get(redh.add_child("red", RMF.ALIAS)).set_aliased(r)
for g in green:
    af.get(greenh.add_child("green", RMF.ALIAS)).set_aliased(g)
for b in blue:
    af.get(blueh.add_child("blue", RMF.ALIAS)).set_aliased(b)
