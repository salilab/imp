## \example container/filter_close_pairs.py
## This example shows how to filter the list of close pairs generated in the IMP.container.ClosePairContainer (or IMP.container.CloseBipartitePairContainer). Eventually the filter should probably be implemented in C++, for speed but implementing the filter in python is good for prototyping.

import IMP
import IMP.container
import IMP.core
import IMP.algebra

np=10
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(5,5,5))
ik= IMP.IntKey("num")
IMP.base.set_log_level(IMP.base.SILENT)
m= IMP.Model()
l= []
for i in range(0, np):
    p= IMP.Particle(m)
    p.add_attribute(ik, i)
    IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 1))
    l.append(p)
lsc= IMP.container.ListSingletonContainer(l)
cpc= IMP.container.ClosePairContainer(lsc, 0.0)

m.update()
print "without",[(x[0].get_name(), x[1].get_name()) for x in cpc.get_particle_pairs()]

class ConsecutiveFilter(IMP.PairPredicate):
    def __init__(self):
        IMP.PairPredicate.__init__(self, "ConsecutiveFilter%1%")
    def get_value(self, pp):
        diff= pp[0].get_value(ik) - pp[1].get_value(ik)
        if diff==-1 or diff ==1:
            return 1
        return 0
    def _do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]
    def do_show(self, out):
        pass
f= ConsecutiveFilter()
cpc.add_pair_filter(f)
m.update()
print "with",[(x[0].get_name(), x[1].get_name()) for x in cpc.get_particle_pairs()]
