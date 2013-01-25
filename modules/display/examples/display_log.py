## \example display/display_log.py
## A simple example of how to use one of the IMP.display.LogOptimizerStates.

import IMP
import IMP.display
import IMP.core
import IMP.container

m= IMP.Model()
ps=IMP.core.create_xyzr_particles(m, 20, 5)
c= IMP.container.ListSingletonContainer(ps)

# write it to a series of files, if the file name did not contain %1%, then
# it would concatenate the outputs into a single file instead. Concatenating the
# output can be quite useful with Pymol as that makes it less likely to crash.
log= IMP.display.WriteOptimizerState(IMP.display.ChimeraWriter("log_file.%1%.pym"))
# the logging occurs ever two frames
log.set_period(2)
g= IMP.core.XYZRsGeometry(c)
g.set_name("my particles")
g.set_color(IMP.display.Color(1,0,0))
log.add_geometry(g)

r= IMP.core.ExcludedVolumeRestraint(c)
m.add_restraint(r)

o= IMP.core.MonteCarlo(m)
mv= IMP.core.BallMover(ps, 10)
o.add_mover(mv)
o.add_optimizer_state(log)

o.optimize(1000)
