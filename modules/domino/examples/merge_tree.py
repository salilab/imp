import IMP
import IMP.domino
import IMP.core
import IMP.container
import IMP.algebra

IMP.set_log_level(IMP.TERSE)
m=IMP.Model()
m.set_log_level(IMP.SILENT)

bb= IMP.algebra.BoundingBox3D((0,0,0), (10,10,10))
allc=[]
for i in range(0,7):
    p= IMP.Particle(m)
    d= IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 1))
    allc.append(d.get_coordinates())
pst= IMP.domino.ParticleStatesTable()
ss= IMP.domino.XYZStates(allc)
for p in m.get_particles():
    pst.set_particle_states(p, ss)
cp= IMP.container.ClosePairContainer(IMP.container.ListSingletonContainer(m.get_particles()),
                                     1, 0)
m.update()
acp= IMP.container.ListPairContainer(cp.get_particle_pairs())
ps= IMP.core.SoftSpherePairScore(1)
r= IMP.container.PairsRestraint(ps, acp)
m.add_restraint(r)

ig= IMP.domino.get_interaction_graph(m.get_root_restraint_set(),
                                     pst)
#ig.show_dotty()
jt= IMP.domino.get_junction_tree(ig)
print dir(jt)
#jt.show_dotty()
print type(jt)
mt= IMP.domino.get_merge_tree(jt)
mt.show_dotty()
s=pst.get_subset()
print s, type(s)
