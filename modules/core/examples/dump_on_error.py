import IMP
import IMP.core

m= IMP.Model()
pts= IMP.core.create_xyzr_particles(m, 10, 10, 10)

IMP.add_failure_handler(IMP.core.DumpModelOnFailure(m, "error.imp"))

#m.remove_particle(pts[0].get_particle())
#m.remove_particle(pts[0].get_particle())
