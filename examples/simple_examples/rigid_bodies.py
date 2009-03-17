import IMP
import IMP.core
import IMP.atom

m= IMP.Model()

mp0= IMP.atom.read_pdb('examples/simple_examples/single_protein.pdb', m)
residues= IMP.atom.get_by_type(mp0, IMP.atom.MolecularHierarchyDecorator.RESIDUE)
rbt= IMP.atom.get_molecular_rigid_body_traits()
rbs=IMP.core.ListSingletonContainer(residues)
IMP.core.create_rigid_bodies(rbs, rbt)

mp1= IMP.atom.read_pdb('examples/simple_examples/single_protein.pdb', m)
residues= IMP.atom.get_by_type(mp1, IMP.atom.MolecularHierarchyDecorator.CHAIN)
rbd=IMP.core.create_rigid_body(residues[0], rbt)
print "all done"
