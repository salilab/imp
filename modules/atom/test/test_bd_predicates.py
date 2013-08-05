import IMP.atom
import IMP.core
import IMP.kernel
import itertools

m = IMP.kernel.Model()

bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                               IMP.algebra.Vector3D(5, 5, 5))

pis = []
for i in range(0, 5):
    pi = m.add_particle("P%d"%i)
    IMP.core.XYZR.setup_particle(m, pi, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 1))
    IMP.core.XYZ(m, pi).set_coordinates_are_optimized(True)
    IMP.atom.Diffusion.setup_particle(m, pi)
    pis.append(pi)

bd = IMP.atom.BrownianDynamics(m)
bd.set_maximum_time_step(1000)
length = IMP.atom.get_diffusion_length(IMP.atom.Diffusion(m, pis[0]).get_diffusion_coefficient(),
                                       bd.get_maximum_time_step())
print "length is", length

ibbsp = IMP.core.InBoundingBox3DSingletonPredicate(bb)
icpp = IMP.core.IsCollisionPairPredicate()

bd.add_move_predicate(icpp, 1, 0, 30)
bd.add_move_predicate(ibbsp, 0)

for pi in pis:
    print IMP.core.XYZ(m, pi)

bd.optimize(100)

for pi in pis:
    print IMP.core.XYZ(m, pi)
    assert(ibbsp.get_value_index(m, pi) == 1)
for ppi in itertools.combinations(pis, 2):
    assert(IMP.core.get_distance(IMP.core.XYZR(m, ppi[0]),
                                 IMP.core.XYZR(m, ppi[1])) > 0)
