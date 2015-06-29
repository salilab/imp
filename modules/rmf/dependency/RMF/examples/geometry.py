## \example geometry.py
# Show creation of geometry in an RMF

from __future__ import print_function
import RMF

tfn = RMF._get_temporary_file_path("aliases.rmf")
print("File is", tfn)

f = RMF.create_rmf_file(tfn)
f.add_frame("root", RMF.FRAME)

r = f.get_root_node()

bf = RMF.BallFactory(f)
cf = RMF.CylinderFactory(f)
sf = RMF.SegmentFactory(f)

b = bf.get(r.add_child("ball", RMF.GEOMETRY))
b.set_radius(1)
b.set_coordinates(RMF.Vector3(0, 0, 0))

c = cf.get(r.add_child("cylinder", RMF.GEOMETRY))
c.set_radius(.5)
c.set_coordinates_list([RMF.Vector3(0, 0, 0), RMF.Vector3(5, 5, 5)])
