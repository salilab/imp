## \example display/basic_geometry.py
## This example shows how to write basic geometric primitives to a file with color and name.

import IMP.display

# or IMP.display.ChimeraWriter
# if using chimera, make sure there is a %1% in the name to support multiple frames
name=IMP.create_temporary_file_name("example", ".py")
print "File name is", name
w= IMP.display.PymolWriter(name)

bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(100, 100, 100))
# we could skip the outer loop if we only have one frame
for f in range(0,10):
    w.set_frame(f)
    g= IMP.display.BoundingBoxGeometry(bb)
    g.set_name("bb")
    w.add_geometry(g)
    for i in range(0,10):
        p= IMP.algebra.get_random_vector_in(bb)
        g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(p, 10))
        # give each a distinctive color
        g.set_color(IMP.display.get_display_color(i))
        g.set_name(str(i))
        # add it to the file
        w.add_geometry(g)
