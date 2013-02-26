## \example core/incremental_mc.py
## This example shows how to do incremental scoring with Monte Carlo. Incremental scoring can be significantly faster than non-incremental scoring when using moves that only move a few particles at a time.

import IMP.core
import IMP.container
import IMP.algebra
import IMP.display
m= IMP.Model()
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(30, 30, 30))
ps=[]
for i in range(0,20):
    ps.append(IMP.Particle(m))
    d=IMP.core.XYZR.setup_particle(ps[-1])
    d.set_radius(10)
    d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
    d.set_coordinates_are_optimized(True)

psl= IMP.container.ListSingletonContainer(ps)
r= IMP.core.ExcludedVolumeRestraint(psl, 1)
rb= IMP.container.SingletonsRestraint(IMP.core.BoundingBox3DSingletonScore(IMP.core.HarmonicUpperBound(0,1), bb),
                                 psl)
mc= IMP.core.MonteCarlo(m)
isf= IMP.core.IncrementalScoringFunction(ps, [r, rb])
mc.set_incremental_scoring_function(isf)

mvs=[IMP.core.BallMover([p], 5) for p in ps]
sm= IMP.core.SerialMover(mvs)
mc.add_mover(sm)
IMP.base.set_log_level(IMP.base.SILENT)
print "initial",isf.evaluate(False)
after=mc.optimize(10000)
print "final", after
name=IMP.base.create_temporary_file_name("incremental_mc", ".pym")
w= IMP.display.PymolWriter(name)
for p in ps:
    g= IMP.core.XYZRGeometry(p)
    w.add_geometry(g)
print "pymol", name
