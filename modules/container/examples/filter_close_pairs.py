## \example container/filter_close_pairs.py
# This example shows how to filter the list of close pairs generated in
# the IMP.container.ClosePairContainer (or
# IMP.container.CloseBipartitePairContainer). Eventually the filter should
# probably be implemented in C++, for speed but implementing the filter in
# Python is good for prototyping.

from __future__ import print_function
import IMP
import IMP.container
import IMP.core
import IMP.algebra
import sys

IMP.setup_from_argv(sys.argv, "filter close pairs")

np = 10
bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                               IMP.algebra.Vector3D(5, 5, 5))
ik = IMP.IntKey("num")
IMP.set_log_level(IMP.SILENT)
m = IMP.Model()
l = []
for i in range(0, np):
    p = m.add_particle("p%d" % i)
    m.add_attribute(ik, p, i)
    IMP.core.XYZR.setup_particle(m,
        p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 1))
    l.append(p)
lsc = IMP.container.ListSingletonContainer(m, l)
cpc = IMP.container.ClosePairContainer(lsc, 0.0)

m.update()
print("without", [(m.get_particle_name(x[0]), m.get_particle_name(x[1]))
                  for x in cpc.get_contents()])


class ConsecutiveFilter(IMP.PairPredicate):

    def __init__(self):
        IMP.PairPredicate.__init__(self, "ConsecutiveFilter%1%")

    def get_value(self, pp):
        diff = pp[0].get_value(ik) - pp[1].get_value(ik)
        if diff == -1 or diff == 1:
            return 1
        return 0

    def do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]

    def do_show(self, out):
        pass
f = ConsecutiveFilter()
cpc.add_pair_filter(f)
m.update()
print("with", [(m.get_particle_name(x[0]), m.get_particle_name(x[1]))
               for x in cpc.get_contents()])
