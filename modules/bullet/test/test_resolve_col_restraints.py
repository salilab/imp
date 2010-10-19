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
                            #print str(si), str(sj), ret, pi.get_name(), pj.get_name()
                            hit=True
                if hit:
                    ret=ret+1
        return ret
    def count_annulus(self, ps):
        ret=0
        for i in range(0, len(ps)):
            d= IMP.core.XYZ(ps[i]).get_coordinates().get_magnitude()
            if d <6 or d > 18:
                ret=ret+1
        return ret

    def create(self):
        m= IMP.Model()
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(5,5,5))
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
        for i in range(0,1):
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
    def test_rcor(self):
        """Test basic ResolveCollision optimization with rigid bodies and restraints"""
        (m, bb, ps)= self.create()
        l= IMP.container.ListSingletonContainer(ps)
        inner= IMP.container.SingletonsRestraint(IMP.core.DistanceToSingletonScore(IMP.core.HarmonicLowerBound(6, 1), IMP.algebra.Vector3D(0,0,0)), l)
        m.add_restraint(inner)
        outer= IMP.container.SingletonsRestraint(IMP.core.DistanceToSingletonScore(IMP.core.HarmonicUpperBound(18, 1), IMP.algebra.Vector3D(0,0,0)), l)
        m.add_restraint(outer)
        print "intesections:", self.count_hits(ps),"annulus:",self.count_annulus(ps)
        opt= IMP.bullet.ResolveCollisionsOptimizer(m)
        opt.set_xyzrs(ps)
        for i in range(0,100):
            self.display(ps, i)
            opt.optimize(1);
            print "intesections:", self.count_hits(ps), "annulus:",self.count_annulus(ps)
        self.assertEqual(self.count_hits(ps), 0)

if __name__ == '__main__':
    IMP.test.main()
