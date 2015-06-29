## \example display/display_log.py
# A simple example of how to use one of the IMP.display.LogOptimizerStates.

import IMP.display
import IMP.core
import IMP.container
import sys

IMP.setup_from_argv(sys.argv,
    "Example of how to use one of the IMP.display.LogOptimizerStates")

m = IMP.Model()
ps = IMP.core.create_xyzr_particles(m, 20, 5)
c = IMP.container.ListSingletonContainer(m, ps)

# Write outputs into a single file.
log = IMP.display.WriteOptimizerState(
    m, IMP.display.PymolWriter("log_file.pym"))
# the logging occurs ever two frames
log.set_period(2)
g = IMP.core.XYZRsGeometry(c)
g.set_name("my particles")
g.set_color(IMP.display.Color(1, 0, 0))
log.add_geometry(g)

r = IMP.core.ExcludedVolumeRestraint(c)
r.set_log_level(IMP.VERBOSE)

o = IMP.core.MonteCarlo(m)
o.set_scoring_function([r])
mv = IMP.core.BallMover(ps, 10)
o.add_mover(mv)
o.add_optimizer_state(log)

o.optimize(1000)
