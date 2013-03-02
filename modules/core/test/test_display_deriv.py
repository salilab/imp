import IMP
import IMP.algebra
import IMP.display
import IMP.core
import IMP.test

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.TERSE)

    def test_derivs(self):
        """Testing execution of derivative display support"""
        # note that there are no actual checks here at this point
        m= IMP.Model()
        pts=[IMP.algebra.Vector3D(1,0,0), IMP.algebra.Vector3D(0,1,0),
             IMP.algebra.Vector3D(-1,0,0), IMP.algebra.Vector3D(0,-1,0)]

        ps= []
        for i in range(0,4):
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p, pts[i])
            ps.append(p)
        p= IMP.Particle(m)
        d= IMP.core.XYZ.setup_particle(p)
        hd= IMP.core.Hierarchy.setup_particle(p, ps)

        for i in range(0,4):
            u= IMP.core.Harmonic(0,1)
            s= IMP.core.DistanceToSingletonScore(u, pts[(i+1)%4])
            r= IMP.core.SingletonRestraint(s, ps[i])
            m.add_restraint(r)

        m.evaluate(True)
        w= IMP.display.BildWriter(self.get_tmp_file_name("deriv.bild"))
        for i in range(0,4):
            w.add_geometry(IMP.core.XYZDerivativeGeometry(IMP.core.XYZ(ps[i])))
        del w

        rbd= IMP.core.RigidBody.setup_particle(p, IMP.core.XYZs(ps))
        IMP.base.set_log_level(IMP.base.TERSE)
        print "eval"
        m.evaluate(True)
        w= IMP.display.BildWriter(self.get_tmp_file_name("qderiv.bild"))
        #oge= display.XYZRGeometryExtractor(FloatKey("hi"))
        for i in range(0,4):
            gs = IMP.core.RigidBodyDerivativeGeometry(rbd)
            w.add_geometry(gs)
            print gs
        del w

if __name__ == '__main__':
    IMP.test.main()
