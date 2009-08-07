import IMP
import IMP.core
import IMP.atom
import IMP.atom
import IMP.helper

m= IMP.Model()
prot= IMP.atom.read_pdb(IMP.get_data_directory()\
                         +'/atom/example_protein.pdb', m)
res= IMP.atom.get_by_type(prot, IMP.atom.Hierarchy.RESIDUE)
rc= IMP.core.ListSingletonContainer(res)
for p in res:
    IMP.core.XYZR.setup_particle(p.get_particle())
mtr=IMP.atom.Hierarchy.get_traits()
pr= IMP.core.ChildrenRefiner(mtr)
IMP.helper.create_covers(rc, pr)
m.evaluate(False)
