import IMP
import IMP.core
import IMP.modeller

m= IMP.Model()
prot= IMP.modeller.read_pdb('single_protein.pdb')
res= IMP.core.molecular_hierarchy_get_by_type(prot, IMP.core.MolecularHierarchyDecorator.RESIDUE)
rc= IMP.core.ListSingletonContainer(res)
for p in res:
    IMP.core.XYZRDecorator.create(p)
ref= IMP.misc.ChildrenParticleRefiner()
cover= IMP.core.CoverRefinedSingletonModifier(ref)
propd= IMP.core.DerivativesToRefinedSingletonModifier(ref)
state=IMP.core.SingletonsState(rc, cover, propd)
m.add_core_state(state)
m.evaluate(False)
