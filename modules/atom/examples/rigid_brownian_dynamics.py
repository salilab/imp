## \example atom/rigid_brownian_dynamics.py
## This example shows how to run brownian dynamics with rigid bodies.

import IMP.atom
import IMP.core
import IMP.algebra
import IMP.display

def create_rigid_body(m, name):
    prb= IMP.core.RigidBody.setup_particle(IMP.Particle(m), IMP.algebra.ReferenceFrame3D())
    prb.set_coordinates_are_optimized(True)
    prb.set_name(name+" rb")
    ph= IMP.atom.Molecule.setup_particle(IMP.Particle(m))
    ph.set_name(name)
    for i in range(0,2):
        for j in range(0,2):
            for k in range(0,2):
                d= IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                                IMP.algebra.Sphere3D(IMP.algebra.Vector3D(i,j,k)*10.0,
                                                                     10))
                ph.add_child(IMP.atom.Fragment.setup_particle(d))
                IMP.atom.Mass.setup_particle(d, 1000)
                prb.add_member(d)
                d.set_name(name+str(i)+str(j)+str(k))
    d= IMP.atom.RigidBodyDiffusion.setup_particle(prb)
    d.set_rotational_diffusion_coefficient(d.get_rotational_diffusion_coefficient()*100)
    return prb, ph

def display(i, w, hs):
    w.set_frame(i)
    for h in hs:
        g= IMP.atom.HierarchyGeometry(h)
        w.add_geometry(g)

m= IMP.Model()
IMP.base.set_log_level(IMP.base.SILENT)

rb0, h0= create_rigid_body(m, "first")
rb1, h1= create_rigid_body(m, "second")
rb1.set_reference_frame(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.Vector3D(30,0,0))))

ev= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(rb0.get_members()+rb1.get_members()),
                                     1, 3)
m.add_restraint(ev)

#h= IMP.core.Harmonic(0,1)
#s= IMP.core.DistanceToSingletonScore(h, IMP.algebra.Vector3D(0,0,0))
#r= IMP.core.SingletonRestraint(s, rb0.get_member(0))
#m.add_restraint(r)
cr= IMP.atom.create_distance_restraint(IMP.atom.Selection(h0), IMP.atom.Selection(h1), 0, 1)
m.add_restraint(cr)

bd=IMP.atom.BrownianDynamics(m)
bd.set_time_step(10000)

nm=IMP.base.create_temporary_file_name("rigid_bd", ".pym")
nm="rigid.pym"
w= IMP.display.PymolWriter(nm)
for i in range(0,100):
    display(i, w, [h0, h1])
    bd.optimize(10)
