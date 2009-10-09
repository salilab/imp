import IMP
import IMP.core
import IMP.atom

m = IMP.Model()
mp0= IMP.atom.read_pdb(IMP.get_data_directory()\
                           +'/atom/example_protein.pdb', m)
# get the 16th residue
chain= mp0.get_child(0)
r16 = IMP.atom.get_residue(chain, 16)
r16.show()

atoms= IMP.atom.get_by_type(mp0, IMP.atom.Hierarchy.ATOM)
print atoms
aa=IMP.atom.Hierarchies()
print aa
print len(atoms)

#get the residues
residues= IMP.atom.get_by_type(mp0, IMP.atom.Hierarchy.AMINOACID)
# set the coordinates and radius of each residue to enclose its atoms
for r in residues:
    d= IMP.core.XYZR.setup_particle(r.get_particle())
    atoms= IMP.atom.get_by_type(IMP.atom.Hierarchy(r),
                                IMP.atom.Hierarchy.ATOM)
    IMP.core.set_enclosing_sphere(d, IMP.core.XYZs(atoms))

# load another copy
mp1= IMP.atom.read_pdb(IMP.get_data_directory() \
                       + '/atom/example_protein.pdb', m)

p = IMP.Particle(m)
rmp= IMP.atom.Hierarchy.setup_particle(p, IMP.atom.Hierarchy.ASSEMBLY)
rmp.steal_children(mp0)
rmp.steal_children(mp1)
