import IMP.em
import IMP.core
import IMP.atom
m= IMP.Model()
378
ps= IMP.Particles()
for i in range(0,15):
    ps.append(IMP.Particle(m))
    d= IMP.core.XYZR.setup_particle(ps[-1],
                                    IMP.algebra.Sphere3D(IMP.algebra.Vector3D(3*i,0,0), 2))
    md= IMP.atom.Mass.setup_particle(ps[-1], 100)
map= IMP.em.read_map(IMP.em.get_example_path("fit_input.mrc"))
map.get_header().set_resolution(3)
r= IMP.em.FitRestraint(ps, map)
m.add_restraint(r)
