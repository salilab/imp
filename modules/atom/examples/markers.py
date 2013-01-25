## \example atom/markers.py
## It is often useful to be able to add extra particles to a model to act as markers for particular features. Examples include creating a bounding sphere for some part of a molecule and using the bounding sphere particle in a distance restraint. The IMP.atom.create_cover() is such a function, creating a particle whose IMP.core.XYZR sphere contains the passed IMP.atom.Selection at all times.
##
## The same code also works (more efficiently in fact) if the protein is made into a rigid body.
##

import IMP.atom
import IMP.display

m= IMP.Model()
h= IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"),
                     m)
# residues 100 and 153 are two residues that are close together
# spatially, create a marker for the volume around them
s= IMP.atom.Selection(h, residue_indexes=[100,153])
marker= IMP.atom.create_cover(s, "my marker")
m.update()

# if we now move the protein, marker will move along with it
rot = IMP.algebra.get_random_rotation_3d()
tr= IMP.algebra.Transformation3D(rot, IMP.algebra.Vector3D(-10,10,10))
IMP.atom.transform(h, tr)
m.update()

# display it to a file
w= IMP.display.PymolWriter("marker.pym")
g= IMP.core.XYZRGeometry(marker)
g.set_color(IMP.display.Color(0,1,0))
gp= IMP.atom.HierarchyGeometry(h)
gs=IMP.atom.SelectionGeometry(s)
gs.set_color(IMP.display.Color(1,0,0))
w.add_geometry(g)
w.add_geometry(gp)
w.add_geometry(gs)
