## \example misc/decay.py
# Use the IMP::misc::DecayPairContainerOptimizerState to gradually break
# the bonds in a bd simulation.

from __future__ import print_function
import IMP.atom
import IMP.container
import IMP.misc
import IMP.display
import IMP.example
import sys
import IMP.rmf
import RMF
import random

IMP.setup_from_argv(
    sys.argv,
    "Use the IMP::misc::DecayPairContainerOptimizerState to gradually "
    "break the bonds in a bd simulation")

if IMP.get_bool_flag("run_quick_test"):
    np = 8
    nb = 8
    prob = .5
    period = 2
    steps = 10
else:
    np = 20
    nb = 40
    prob = .9
    period = 10
    steps = 10000

m = IMP.Model()

bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                               IMP.algebra.Vector3D(100, 100, 100))
ps = []
for i in range(0, np):
    p = m.add_particle("p")
    d = IMP.core.XYZR.setup_particle(m, p)
    d.set_coordinates(
        IMP.algebra.Vector3D(10. * (i / 10), 10. * (i % 10), 10.))
    d.set_radius(10)
    d.set_coordinates_are_optimized(True)
    IMP.atom.Hierarchy.setup_particle(m, p)
    IMP.atom.Diffusion.setup_particle(m, p)
    IMP.atom.Mass.setup_particle(m, p, 1)
    ps.append(p)
    IMP.display.Colored.setup_particle(m, p, IMP.display.get_display_color(i))

bds = []
for i in range(0, nb):
    pp = random.sample(ps, 2)
    if pp not in bds and [pp[1], pp[0]] not in bds:
        bds.append(pp)

cf = IMP.core.CoinFlipPairPredicate(prob)
dos = IMP.misc.DecayPairContainerOptimizerState(m, cf, bds, "decay")
dos.set_period(period)
dos.set_log_level(IMP.SILENT)  # VERBOSE

# create restraints
rs = []
box_score = IMP.core.BoundingBox3DSingletonScore(
    IMP.core.HarmonicUpperBound(0, 10), bb)
ps_container = IMP.container.ListSingletonContainer(m, ps)
rs.append(IMP.container.SingletonsRestraint(box_score, ps_container, "box"))
bond_score = IMP.core.HarmonicUpperBoundSphereDistancePairScore(0, 10)
rs.append(IMP.container.PairsRestraint(bond_score,
                                       dos.get_output_container(),
                                       "bonds"))
ev = IMP.core.ExcludedVolumeRestraint(ps_container, 10, 10)
IMP.set_log_level(IMP.SILENT)

# set up simulator
bd = IMP.atom.BrownianDynamics(m)
bd.set_maximum_time_step(1000)
bd.set_scoring_function(rs + [ev])
bd.add_optimizer_state(dos)

# set up display
fn = IMP.create_temporary_file_name("decay", ".rmf")
rmf = RMF.create_rmf_file(fn)
print("setting up file")
IMP.rmf.add_hierarchies(rmf, IMP.get_particles(m, ps))
IMP.rmf.add_restraints(rmf, rs + [ev])
g = IMP.display.BoundingBoxGeometry(bb)
IMP.rmf.add_geometries(rmf, [g])
os = IMP.rmf.SaveOptimizerState(m, rmf)
os.set_period(max(steps / 100, 1))
bd.add_optimizer_state(os)

# actually optimize things
print("running")
bd.optimize(steps)

print("see", fn)
