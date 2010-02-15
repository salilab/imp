import IMP
import IMP.display
import IMP.core

m= IMP.Model()
c= IMP.container.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 20, 5))

log= IMP.display.LogOptimizerState(IMP.display.PymolWriter(), "log_file.%04d.pym")
g= IMP.display.XYZRsGeometry(c, IMP.core.XYZR.get_default_radius_key())
g.set_name("my particles")
g.set_color(IMP.display.Color(1,0,0))
log.add_geometry(g)
log.write("initial.pym")

r= IMP.core.ExcludedVolumeRestraint(c)
m.add_restraint(r)

o= IMP.core.MonteCarlo(m)
mv= IMP.core.BallMover(c, 10)
o.add_mover(mv)
o.add_optimizer_state(log)

o.optimize(1000)
