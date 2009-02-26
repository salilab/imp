import IMP
import IMP.core
import IMP.modeller

m= IMP.Model()
prot= IMP.modeller.read_pdb('examples/simple_examples/single_protein.pdb', m)
res= IMP.core.get_by_type(prot, IMP.core.MolecularHierarchyDecorator.RESIDUE)
rc= IMP.core.ListSingletonContainer(res)
for p in res:
    IMP.core.XYZRDecorator.create(p)
ref= IMP.core.ChildrenParticleRefiner(IMP.core.MolecularHierarchyDecorator.get_traits())
cover= IMP.core.CoverRefined(ref)
propd= IMP.core.DerivativesToRefined(ref)
state=IMP.core.SingletonsScoreState(rc, cover, propd)
m.add_score_state(state)
m.evaluate(False)
