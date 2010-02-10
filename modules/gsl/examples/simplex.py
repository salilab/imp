import IMP, IMP.test
import IMP.core
import IMP.gsl
import IMP.algebra


m= IMP.Model()
d0= IMP.core.XYZ.setup_particle(IMP.Particle(m), IMP.algebra.Vector3D(0,0,0))
d1= IMP.core.XYZ.setup_particle(IMP.Particle(m), IMP.algebra.Vector3D(3,4,5))
d0.set_coordinates_are_optimized(True)
d1.set_coordinates_are_optimized(True)

dist= IMP.core.DistanceRestraint(IMP.core.Harmonic(1, 1), d0, d1)
m.add_restraint(dist)

opt = IMP.gsl.Simplex(m)
opt.set_minimum_size(.000001)
opt.set_initial_length(1)
e = opt.optimize(1000000)
print IMP.core.distance(d0, d1)
