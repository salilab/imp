import IMP
import IMP.core
import IMP.atom
import IMP.atom
import IMP.helper

m= IMP.Model()
prot= IMP.atom.read_pdb(IMP.core.get_example_path('example_protein.pdb'), m)
res= IMP.atom.get_by_type(prot, IMP.atom.RESIDUE_TYPE)
for p in res:
    IMP.core.XYZR.setup_particle(p.get_particle())
mtr=IMP.atom.Hierarchy.get_traits()
pr= IMP.core.ChildrenRefiner(mtr)
for r in res:
    IMP.core.Cover.setup_particle(r.get_particle(), pr)
m.evaluate(False)
