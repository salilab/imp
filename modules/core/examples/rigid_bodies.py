import IMP
import IMP.core
import IMP.atom
import IMP.helper

m= IMP.Model()

mp0= IMP.atom.read_pdb(IMP.atom.get_example_path('example_protein.pdb'), m)
residues= IMP.atom.get_by_type(mp0, IMP.atom.RESIDUE_TYPE)
rbs=IMP.core.ListSingletonContainer(residues)
for r in residues:
    IMP.core.RigidBody.setup_particle(r.get_particle(), IMP.core.XYZs(IMP.atom.get_leaves(r)))

mp1= IMP.atom.read_pdb(IMP.atom.get_example_path('example_protein.pdb'), m)
chains= IMP.atom.get_by_type(mp1, IMP.atom.CHAIN_TYPE)
rd= IMP.atom.Hierarchy(chains[0])
rbs=IMP.atom.rigid_body_setup_hierarchy(chains[0])
print "all done"
