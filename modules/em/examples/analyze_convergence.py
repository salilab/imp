import IMP.display
import IMP.em

use_rigid_bodies=True
bd= 10
radius=10

m= IMP.Model()
p= IMP.Particle(m)
IMP.atom.Mass.setup_particle(p, 10000)
d= IMP.core.XYZR.setup_particle(p)
d.set_radius(radius)

# Set up the particle as either a rigid body or a simple ball
if use_rigid_bodies:
    prb= IMP.Particle(m)
    prb.set_name("rigid body")
    d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
    drb= IMP.core.RigidBody.setup_particle(prb, [p], IMP.algebra.ReferenceFrame3D())
    print "initial frame", drb.get_reference_frame()
    fp= prb
    drb.set_coordinates_are_optimized(True)
    refiner= IMP.core.TableRefiner()
    refiner.add_particle(prb, [p])
    print [p.get_name() for p in refiner.get_refined(prb)]
else:
    fp= d
    d.set_coordinates_are_optimized(True)
    refiner=None


bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-bd-radius, -bd-radius, -bd-radius),
                              IMP.algebra.Vector3D( bd+radius,  bd+radius,  bd+radius))

dheader = IMP.em.create_density_header(bb,1)
dheader.set_resolution(1)
dmap = IMP.em.SampledDensityMap(dheader)
dmap.set_particles([p])

dmap.resample()
# computes statistic stuff about the map and insert it in the header
dmap.calcRMS()
IMP.em.write_map(dmap,"map.mrc",IMP.em.MRCReaderWriter())
rs= IMP.RestraintSet()
m.add_restraint(rs)
#rs.set_weight(.003)

# if rigid bodies are used, we need to define a refiner as
# FitRestraint doesn't support just passing all the geometry
r= IMP.em.FitRestraint([fp], dmap, refiner)
rs.add_restraint(r)
g= IMP.display.XYZDerivativeGeometry(d)
g.set_name("deriv")
w= IMP.display.PymolWriter("derivatives.pym")
# kind of abusive
steps=4
m.set_log_level(IMP.SILENT)

opt= IMP.core.ConjugateGradients(m)
for i in range(-bd, bd+1, 2*bd/steps):
    for j in range(-bd, bd+1, 2*bd/steps):
        for k in range(-bd, bd+1, 2*bd/steps):
            print i,j,k
            vc=IMP.algebra.Vector3D(i,j,k)
            d.set_coordinates(vc)
            # display the score at this position
            cg= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(vc, 1))
            cg.set_name("score")
            v=m.evaluate(True)
            cg.set_color(IMP.display.get_hot_color(v))
            w.add_geometry(cg)
            print v, d.get_derivatives()
            w.add_geometry(g)

            opt.optimize(10)
            print "after", d.get_coordinates()
            mag= d.get_coordinates().get_magnitude()

            converge_color= IMP.display.get_grey_color(1.0/(1.0+mag))
            # display the distance after optimization at this position
            g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(vc, 1))
            g.set_color(converge_color)
            g.set_name("converge")
            w.add_geometry(g)
