## \example domino/merge_tree.py
## The example shows how to generate and inspect a merge tree for use in Domino.

import IMP
import IMP.domino
import IMP.core
import IMP.container
import IMP.algebra

IMP.base.set_log_level(IMP.base.TERSE)
m=IMP.Model()
# don't print messages about evaluation
m.set_log_level(IMP.base.SILENT)

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
# generate a set of restraints based on the close pairs in this randomly chosen configuration
cp= IMP.core.GridClosePairsFinder()
cp.set_distance(1)
cps=cp.get_close_pairs(m.get_particles())

if len(cps)>0:
    # one cannot create a container from an empty list
    acp= IMP.container.ListPairContainer(cps)
else:
    acp= IMP.container.ListPairContainer(m)
ps= IMP.core.SoftSpherePairScore(1)
r= IMP.container.PairsRestraint(ps, acp)
r.set_model(m)

# compute the interaction graph based on all the restraints
ig= IMP.domino.get_interaction_graph([r],
                                     pst)
# generate a junction tree from the interaction graph
jt= IMP.domino.get_junction_tree(ig)
print dir(jt)
print type(jt)
# create a merge tree from the junction tree, this can be passed to IMP.domin.DominoSampler
mt= IMP.domino.get_merge_tree(jt)
s=pst.get_subset()
print s, type(s)
