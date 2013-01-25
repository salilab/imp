## \example core/rigid_collisions.py
## Collision detection and building of a non-bonded list can be sped up when rigid bodies are used.
## To do this, use a RigidClosePairsFinder.

import IMP
import IMP.core
import IMP.atom
import IMP.container

# This example addes a restraint on nonbonded interactions
# Since it is between two rigid bodies, internal interactions are ignored

m= IMP.Model()
# The particles in the rigid bodies
rbps0= IMP.core.create_xyzr_particles(m, 3, 1)
rbps1= IMP.core.create_xyzr_particles(m, 3, 1)

rbp0= IMP.Particle(m)
rbp1= IMP.Particle(m)

rbss0 = IMP.core.RigidBody.setup_particle(rbp0, IMP.core.XYZs(rbps0))

rbss1 = IMP.core.RigidBody.setup_particle(rbp1, IMP.core.XYZs(rbps1))

lsc= IMP.container.ListSingletonContainer(m)
lsc.add_particles(rbps0)
lsc.add_particles(rbps1)

# Set up the nonbonded list
nbl= IMP.container.ClosePairContainer(lsc, 0, IMP.core.RigidClosePairsFinder(), 2.0)

# Set up excluded volume
ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1))
evr= IMP.container.PairsRestraint(ps, nbl)
evri= m.add_restraint(evr)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

done=False
while not done:
    try:
        o.optimize(1000)
    except IMP.base.ModelException:
        for d in [rbss0, rbss1]:
            d.set_transformation(IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                                              IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                                                                               IMP.algebra.Vector3D(10,10,10)))))
    else:
        done=True
