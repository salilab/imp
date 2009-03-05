import IMP
import IMP.core
import IMP.atom

m= IMP.Model()
prot= IMP.atom.read_pdb('examples/simple_examples/single_protein.pdb', m)
res= IMP.atom.get_by_type(prot, IMP.atom.MolecularHierarchyDecorator.RESIDUE)
rc= IMP.core.ListSingletonContainer(res)
for p in res:
    IMP.core.XYZRDecorator.create(p)
mtr=IMP.atom.MolecularHierarchyDecorator.get_traits()
pr= IMP.core.ChildrenParticleRefiner(mtr)
IMP.core.setup_covers(m, rc, pr)
m.evaluate(False)
