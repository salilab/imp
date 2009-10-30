import IMP
import IMP.core
import IMP.atom
import IMP.atom
import IMP.helper

m= IMP.Model()
prot= IMP.atom.read_pdb(IMP.get_data_directory()\
                         +'/atom/example_protein.pdb', m)
res= IMP.atom.get_by_type(prot, IMP.atom.RESIDUE_TYPE)
rc= IMP.core.ListSingletonContainer(res)
for p in res:
    IMP.core.XYZR.setup_particle(p.get_particle())
mtr=IMP.atom.Hierarchy.get_traits()
pr= IMP.core.ChildrenRefiner(mtr)
for r in rc.get_particles:
    IMP.core.Cover.setup_particle(r, pr)
m.evaluate(False)
