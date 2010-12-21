import IMP
import IMP.container
import IMP.core
import IMP.algebra

np=10
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(5,5,5))
ik= IMP.IntKey("num")
IMP.set_log_level(IMP.SILENT)
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

class ConsecutiveFilter(IMP.PairFilter):
    def __init__(self):
        IMP.PairFilter.__init__(self)
    def get_contains_particle_pair(self, pp):
        diff= pp[0].get_value(ik)-pp[1].get_value(ik)
        if diff==-1 or diff ==1:
            return True
        return False
    def get_input_particles(self, p):
        return [p]
    def get_input_containers(self, p):
        return []
    def do_show(self, out):
        pass
f= ConsecutiveFilter()
cpc.add_pair_filter(f)
m.update()
print "with",[(x[0].get_name(), x[1].get_name()) for x in cpc.get_particle_pairs()]
