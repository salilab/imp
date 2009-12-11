import IMP
import IMP.core
import IMP.atom

m = IMP.Model()
mp0= IMP.atom.read_pdb(IMP.atom.get_example_path('example_protein.pdb'), m)
# get the 16th residue
chain= mp0.get_child(0)
r16 = IMP.atom.get_residue(chain, 16)
r16.show()

atoms= IMP.atom.get_by_type(mp0, IMP.atom.ATOM_TYPE)
print atoms
aa=IMP.atom.Hierarchies()
print aa
print len(atoms)

#get the residues
residues= IMP.atom.get_by_type(mp0, IMP.atom.RESIDUE_TYPE)
# set the coordinates and radius of each residue to enclose its atoms
for r in residues:
    d= IMP.core.XYZR.setup_particle(r.get_particle())
    atoms= IMP.atom.get_by_type(IMP.atom.Hierarchy(r),
                                IMP.atom.ATOM_TYPE)
    IMP.core.set_enclosing_sphere(d, IMP.core.XYZs(atoms))

# load another copy
mp1= IMP.atom.read_pdb(IMP. atom.get_example_path('example_protein.pdb'), m)

p = IMP.Particle(m)
rmp= IMP.atom.Hierarchy.setup_particle(p)
rmp.add_child(mp0)
rmp.add_child(mp1)
