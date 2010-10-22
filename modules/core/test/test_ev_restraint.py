import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.container
import math

class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def _setup_ev_restraint(self):
        #IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        print "read"
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        print 'create'
        r0s= IMP.core.RigidBody.setup_particle(p0.get_particle(),
                                        IMP.core.XYZs(IMP.core.get_leaves(p0)))
        print "radius"
        for p in IMP.core.get_leaves(p0):
            #print p.get_particle().get_name()
            d= IMP.core.XYZR.setup_particle(p.get_particle())
            d.set_radius(1)
        rb0= IMP.core.RigidBody(p0.get_particle())
        rb0.set_coordinates_are_optimized(True)
        sc= IMP.container.ListSingletonContainer(m)
        fps=[]
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(10)
            sc.add_particle(p)
            fps.append(d)
            d.set_coordinates_are_optimized(True)
        for p in sc.get_particles():
            d= IMP.core.XYZ(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                                               IMP.algebra.Vector3D(5,5,5))))
        sc.add_particles(IMP.core.get_leaves(p0))
        r= IMP.core.ExcludedVolumeRestraint(sc)
        r.set_log_level(IMP.SILENT)
        m.add_restraint(r)
        return (m, r, sc)
    def test_ev(self):
        """Testing excluded volume restraint"""
        (m,r, sc)= self._setup_ev_restraint();
        print "mc"
        o= IMP.core.MonteCarlo()
        # very dumb op
        bm= IMP.core.BallMover(sc, IMP.core.XYZ.get_xyz_keys(), 100)
        o.add_mover(bm)
        o.set_model(m)
        print "opt"
        print o.optimize(10)
        print "inspect"
        for p in sc.get_particles():
            for q in sc.get_particles():
                if p==q:
                    continue
                if IMP.core.RigidMember.particle_is_instance(p) \
                   and IMP.core.RigidMember.particle_is_instance(q) \
                   and IMP.core.RigidMember(p).get_rigid_body()\
                   == IMP.core.RigidMember(q).get_rigid_body():
                    continue
                print p.get_name(), q.get_name()
                d= IMP.core.get_distance(IMP.core.XYZR(p), IMP.core.XYZR(q))
                self.assertGreater(d, -.1)
    def test_isolated_ev(self):
        """Testing isolated evaluation of ev restraint"""
        (m,r, sc)= self._setup_ev_restraint()
        IMP.set_log_level(IMP.VERBOSE)
        v= r.evaluate(False)
        self.assertNotEqual(v, 0)
if __name__ == '__main__':
    IMP.test.main()
