import IMP
import IMP.core
import IMP.atom
import IMP.helper
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

lsc= IMP.container.ListSingletonContainer()
lsc.add_particle(rbp0)
lsc.add_particle(rbp1)

tr= IMP.core.TableRefiner()
tr.add_particle(rbp0, rbps0)
tr.add_particle(rbp1, rbps1)

# Set up the nonbonded list
nbl= IMP.container.ClosePairContainer(lsc, 0, IMP.core.RigidClosePairsFinder(tr), 2.0)

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
    except IMP.ModelException:
        for d in [rbss0, rbss1]:
            d.set_transformation(IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                                              IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(0,0,0),
                                                                                               IMP.algebra.Vector3D(10,10,10))))
    else:
        done=True
