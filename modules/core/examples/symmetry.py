## \example core/symmetry.py
## Show how to use the code in core to enforce symmetry.

import IMP.core
import IMP.container

m= IMP.Model()
m.set_log_level(IMP.base.SILENT)
ps =[]
# create 4 xyz particles
for i in range(0,4):
    p = IMP.Particle(m)
    ps.append(p)
    d= IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(.5,0,0))
    IMP.core.XYZR.setup_particle(p, 1)

# set the 0 particle as the reference particle for the others as
# they will get their positions from it
for i,p in enumerate(ps[1:]):
    # the other 3 particles are all symmetric copies of the first
    IMP.core.Reference.setup_particle(p, ps[0])
    # the symmetry operation is rotation around the z axis
    tr= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.get_basis_vector_3d(2),
                                                                         3.14/2.0*(i+1)),
                                     IMP.algebra.Vector3D(0,0,0))
    sm= IMP.core.TransformationSymmetry(tr)
    # set up a constraint for the one particle, if you have more than one with the same symmetry
    # transform, you should use an IMP.container.SingletonsConstraint.
    c= IMP.core.SingletonConstraint(sm, None, p)
    m.add_score_state(c)
r= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(ps),
                                    1)
m.add_restraint(r)

d0= IMP.core.XYZ(ps[0])
# print only optimize the main particle
d0.set_coordinates_are_optimized(True)

opt= IMP.core.ConjugateGradients(m)
opt.optimize(10)
print "score is ", m.evaluate(False)
for p in ps:
    print p.get_name(), IMP.core.XYZ(p).get_coordinates()
