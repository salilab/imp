## \example algebra/geometry.py
## IMP.algebra provides a set of geometric primitives and basic operations on them.

import IMP.algebra

# we can create some spheres
s=[]
for i in range(0,10):
    s.append(IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()), .1))

# we can compute a sphere which contains them all
enclosing= IMP.algebra.get_enclosing_sphere(s)

print enclosing.get_contains(s[0])

print IMP.algebra.get_distance(s[0], s[1])
# or between the centers
print IMP.algebra.get_distance(s[0].get_center(), s[1].get_center())

# create a cylinder
c= IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(s[0].get_center(), s[1].get_center()), 1)
print c

# manipulate bounding boxes
bb= IMP.algebra.BoundingBox3D()
for si in s:
    bb+= IMP.algebra.get_bounding_box(si)
