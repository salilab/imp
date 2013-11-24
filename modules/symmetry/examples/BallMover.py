## \example BallMover.py
## Symmetry ball mover

import IMP
import IMP.symmetry
import IMP.core


# parameters
S_ = 20.      # cubic cell size
K_ = 1.0      # intensity harmonic restraint
KBT_ = 1.0    # temperature

# create model
m = IMP.kernel.Model()

# add 2 particles
ps = []
for i in range(2):
    p = IMP.kernel.Particle(m)
    d = IMP.core.XYZR.setup_particle(p,
                                     IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0., 0., 0.), 1.0))
    d.set_coordinates_are_optimized(True)
    ps.append(p)

# add harmonic restraint to distance
h = IMP.core.Harmonic(0.0, K_)
sps = IMP.core.SphereDistancePairScore(h)
pr = IMP.core.PairRestraint(sps, IMP.kernel.ParticlePair(ps[0], ps[1]))
m.add_restraint(pr)

# define cell centers
ctrs = []
for i in [0.0, -1.0, +1.0]:
    for j in [0.0, -1.0, +1.0]:
        for k in [0.0, -1.0, +1.0]:
            ctrs.append(IMP.algebra.Vector3D(
                float(i) * S_, float(j) * S_, float(k) * S_))

# define transformation from primitive to all cells
trs = []
for ctr in ctrs:
    trs.append(IMP.algebra.Transformation3D(ctr))

# movers
movers = []
# symmetry mover with ps[0] being the master particle
movers.append(IMP.symmetry.BallMover(ps[0], [ps[1]], 1.0, ctrs, trs))
# normal BallMover for the other particle
movers.append(IMP.core.BallMover([ps[1]], 1.0))
# serial mover
sm = IMP.core.SerialMover(movers)

# sampler
mc = IMP.core.MonteCarlo(m)
mc.set_kt(KBT_)
mc.set_return_best(False)
mc.add_mover(sm)

# prepare output
log = open("traj.xyz", "w")

# start sampling loop
for istep in range(0, 5000):
    # do optimization
    mc.optimize(10)

    # coordinates
    xyz0 = IMP.core.XYZR(ps[0]).get_coordinates()
    xyz1 = IMP.core.XYZR(ps[1]).get_coordinates()

    # print
    log.write("\n2\n")
    log.write("Na %6.3f %6.3f %6.3f\n" % (xyz0[0], xyz0[1], xyz0[2]))
    log.write("Cl %6.3f %6.3f %6.3f\n" % (xyz1[0], xyz1[1], xyz1[2]))
