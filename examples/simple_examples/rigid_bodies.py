import IMP
import IMP.core
import IMP.modeller

m= IMP.Model()

cpr= IMP.core.ChildrenParticleRefiner(IMP.core.MolecularHierarchyDecorator.get_traits())
mp0= IMP.modeller.read_pdb('examples/simple_examples/single_protein.pdb', m)
residues= IMP.core.get_by_type(mp0, IMP.core.MolecularHierarchyDecorator.RESIDUE)
rbt= IMP.core.RigidBodyTraits("residues")
rbs=IMP.core.ListSingletonContainer()
for r in residues:
    IMP.core.XYZDecorator.create(r)
    rb=IMP.core.RigidBodyDecorator.create(r, cpr, rbt)
    rbs.add_particle(rb.get_particle())
    print r
print "done with setup"

IMP.core.setup_rigid_bodies(m, rbs, cpr, rbt)
print "all done"
