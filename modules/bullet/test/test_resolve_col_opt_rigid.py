import IMP
import IMP.test
import IMP.bullet
import IMP.core
import IMP.algebra
import IMP.display
import StringIO
import math

class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def count_hits(self, ps):
        ret=0;
        for i in range(0, len(ps)):
            for j in range(0,i):
                pi= ps[i]
                pj= ps[j]
                if IMP.core.RigidMember.particle_is_instance(pi)\
                   and IMP.core.RigidMember.particle_is_instance(pj)\
                   and IMP.core.RigidMember(pi).get_rigid_body() == IMP.core.RigidMember(pj).get_rigid_body():
                    continue
                sis= [ IMP.core.XYZR(pi).get_sphere()]
                sjs= [ IMP.core.XYZR(pj).get_sphere()]
                hit=False
                for si in sis:
                    for sj in sjs:
                        if IMP.algebra.get_distance(si, sj) < 0:
                            print str(si), str(sj), ret, pi.get_name(), pj.get_name()
                            hit=True
                if hit:
                    ret=ret+1
        return ret
    def create(self):
        m= IMP.Model()
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(3,3,3))
        ps=[]
        rps=[]
        for i in range(0,0):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)
            ps.append(p)
            rps.append(p)
            d.set_radius(2)
            print d
        for i in range(0,3):
            rbp= IMP.Particle(m)
            rbp.set_name("rb"+str(i))
            rbps=[]
            rps.append(rbp)
            ls= None
            for j in range(0,10):
                p= IMP.Particle(m)
                p.set_name("rm"+str(i))
                d= IMP.core.XYZR.setup_particle(p)
                d.set_radius(.9)
                if not ls:
                    d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
                else:
                    d.set_coordinates(IMP.algebra.get_random_vector_on(ls))
                    print IMP.algebra.get_distance(d.get_coordinates(), ls.get_center())
                ls= d.get_sphere()
                d.set_coordinates_are_optimized(False)
                rbps.append(p)
                ps.append(d);
            IMP.core.RigidBody.setup_particle(rbp, rbps).set_coordinates_are_optimized(True)
        r= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(ps))
        m.add_restraint(r)
        return (m, bb, ps)
    def display(self, ps, i):
        w= IMP.display.PymolWriter(self.get_tmp_file_name("rigcol."+str(i)+".pym"))
        for i,p in enumerate(ps):
            g= IMP.display.XYZRGeometry(IMP.core.XYZR(p))
            g.set_color(IMP.display.get_display_color(i))
            w.add_geometry(g)
    def _test_rcos(self):
        """Test basic ResolveCollision optimization with rigid bodies"""
        (m, bb, ps)= self.create()
        print "intesections:", self.count_hits(ps),"score:", m.evaluate(False)
        opt= IMP.bullet.ResolveCollisionsOptimizer(m)
        opt.set_xyzrs(ps)
        for i in range(0,100):
            self.display(ps, i)
            opt.optimize(100);
            print "intesections:", self.count_hits(ps), "score:", m.evaluate(False)
        self.display(ps, i)
        self.assertEqual(self.count_hits(ps), 0)
        self.assertLess(m.evaluate(False), .01)
    def test_rcos_local(self):
        """Test local ResolveCollision optimization with rigid bodies"""
        (m, bb, ps)= self.create()
        print "intesections:", self.count_hits(ps),"score:", m.evaluate(False)
        opt= IMP.bullet.ResolveCollisionsOptimizer(m)
        opt.set_local_stiffness(.01)
        opt.set_xyzrs(ps)
        self.display(ps, 0)
        opt.optimize(0)
        self.display(ps, 1)
        for i in range(0,10):
            self.display(ps, i+2)
            opt.optimize(100);
            print "intesections:", self.count_hits(ps), "score:", m.evaluate(False)
        self.display(ps, i+3)
        self.assertEqual(self.count_hits(ps), 0)
        self.assertLess(m.evaluate(False), .01)


if __name__ == '__main__':
    IMP.test.main()
