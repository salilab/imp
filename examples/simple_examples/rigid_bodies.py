import IMP
import IMP.core
import IMP.atom

m= IMP.Model()

mp0= IMP.atom.read_pdb('examples/simple_examples/single_protein.pdb', m)
residues= IMP.atom.get_by_type(mp0, IMP.atom.MolecularHierarchyDecorator.RESIDUE)
rbs=IMP.core.ListSingletonContainer(residues)
IMP.core.create_rigid_bodies(rbs, IMP.core.LeavesRefiner(IMP.atom.MolecularHierarchyDecorator.get_traits()))

mp1= IMP.atom.read_pdb('examples/simple_examples/single_protein.pdb', m)
chains= IMP.atom.get_by_type(mp1, IMP.atom.MolecularHierarchyDecorator.CHAIN)
rd= IMP.atom.MolecularHierarchyDecorator(chains[0])
rbd=IMP.core.create_rigid_body(chains[0], IMP.atom.get_by_type(mp1, IMP.atom.MolecularHierarchyDecorator.ATOM))
print "all done"
