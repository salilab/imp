## \example rmf/simulation.py
## This example shows writing a brownian dynamics simulation to a rmf file. It includes a bounding box, restraints and a hierarchy of particles.

import IMP.atom
import IMP.rmf
import RMF
import IMP.container
import IMP.display

k=10.0
np=10
frames=10

m= IMP.Model()
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))

ps= [IMP.core.XYZR.setup_particle(IMP.Particle(m)) for i in range(0,np)]
h= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m, "root"))
for p in ps:
    p.set_radius(1)
    p.set_coordinates_are_optimized(True)
    IMP.atom.Mass.setup_particle(p, 30)
    IMP.atom.Diffusion.setup_particle(p)
    p.set_coordinates(IMP.algebra.get_random_vector_in(bb))
rs=[]
link= IMP.core.HarmonicUpperBoundSphereDistancePairScore(0,k)
pl=[]
for i in range(0, len(ps)/2):
    pp=(ps[2*i], ps[2*i+1])
    pl.append(pp)
    # create OK staring position
    pp[1].set_coordinates(IMP.algebra.get_random_vector_on(pp[0].get_sphere()))
    IMP.display.Colored.setup_particle(pp[0], IMP.display.get_display_color(i))
    IMP.display.Colored.setup_particle(pp[1], IMP.display.get_display_color(i))
    hr= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m, "molecule "+str(i)))
    hr.add_child(IMP.atom.Hierarchy.setup_particle(pp[0]))
    hr.add_child(IMP.atom.Hierarchy.setup_particle(pp[1]))
    h.add_child(hr)
r= IMP.container.PairsRestraint(link, pl, "Bonds")
rs.append(r)

ev= IMP.core.ExcludedVolumeRestraint(ps, k, 1, "EV")
rs.append(ev)

bbss= IMP.core.BoundingBox3DSingletonScore(IMP.core.HarmonicUpperBound(0,k), bb)
bbr= IMP.container.SingletonsRestraint(bbss, ps)
rs.append(bbr)

sf= IMP.core.RestraintsScoringFunction(rs, "SF")

bd= IMP.atom.BrownianDynamics(m)
bd.set_log_level(IMP.base.SILENT)
bd.set_scoring_function(sf)

bd.set_maximum_time_step(10)

name= IMP.base.create_temporary_file_name("brownian", ".rmf")
rmf= RMF.create_rmf_file(name)
rmf.set_description("Brownian dyanmics trajectory with 10fs timestep.\n")
IMP.rmf.add_hierarchy(rmf, h)
IMP.rmf.add_restraints(rmf, rs)
IMP.rmf.add_geometry(rmf, IMP.display.BoundingBoxGeometry(bb))

os= IMP.rmf.SaveOptimizerState(rmf)
os.update_always("initial conformation")
os.set_log_level(IMP.base.SILENT)
os.set_simulator(bd)
bd.add_optimizer_state(os)

bd.optimize(frames)

print "file is", rmf.get_name()
