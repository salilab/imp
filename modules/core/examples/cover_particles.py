## \example core/cover_particles.py
## Show how to maintain a sphere per residue which includes all atoms of the residue. The derivatives are propagated from the sphere cover to the atoms so that restraints can be used at multiple levels.

import IMP
import IMP.core
import IMP.atom
import IMP.atom

m= IMP.Model()
prot= IMP.atom.read_pdb(IMP.core.get_example_path('example_protein.pdb'), m)
res= IMP.atom.get_by_type(prot, IMP.atom.RESIDUE_TYPE)
pr= IMP.core.ChildrenRefiner(IMP.atom.Hierarchy.get_traits())
for r in res:
    # add coordinates and a radius to the residue particle
    IMP.core.XYZR.setup_particle(r)
    # make sure that the coordinates and the radius define a sphere
    # which contains all of the leaves (atoms) of the residue
    IMP.core.Cover.setup_particle(r.get_particle(), pr)
# update the coordinates of the residue particles so that they cover the atoms
m.update()
