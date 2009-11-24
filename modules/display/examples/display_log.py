import IMP
import IMP.display
import IMP.core

m= IMP.Model()
c= IMP.core.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 20, 5))

log= IMP.display.LogOptimizerState(IMP.display.PymolWriter(), "log_file.%04d.pym")
g= IMP.display.XYZRsGeometry(c)
g.set_name("my particles")
g.set_color(IMP.display.Color(1,0,0))
log.add_geometry(g)
log.write("initial.pym")

r= IMP.core.ExcludedVolumeRestraint(c)
m.add_restraint(r)

o= IMP.core.ConjugateGradients()
o.set_model(m)
o.add_optimizer_state(log)

done=False
while not done:
    try:
        o.optimize(1000)
    except IMP.ModelException:
        for d in [IMP.core.XYZ(x) for x in c.get_particles()]:
            d.set_coordinates(IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(0,0,0),
                                                               IMP.algebra.Vector3D(10,10,10)))
    else:
        done=True
