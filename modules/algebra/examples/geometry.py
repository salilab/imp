## \example modules/algebra/examples/geometry.py
## IMP.algebra provides a set of geometric primitives and basic operations on them.

from IMP.algebra import *

# we can create some spheres
s=[]
for i in range(0,10):
    s.append(Sphere3D(get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()), .1))

# we can compute a sphere which contains them all
enclosing= get_enclosing_sphere(s)

print enclosing.get_contains(s[0])

print IMP.algebra.get_distance(s[0], s[1])
# or between the centers
print get_distance(s[0].get_center(), s[1].get_center())

# create a cylinder
c= Cylinder3D(Segment3D(s[0].get_center(), s[1].get_center()), 1)
print c

# manipulate bounding boxes
bb= BoundingBox3D()
for si in s:
    bb+= get_bounding_box(si)
