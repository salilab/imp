import IMP
import IMP.algebra
import IMP.display
import IMP.core
import IMP.test
import IMP.helper
import unittest

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def test_derivs(self):
        """Testing execution of derivative display support"""
        # note that there are no actual checks here at this point
        m= IMP.Model()
        pts=[IMP.algebra.Vector3D(1,0,0), IMP.algebra.Vector3D(0,1,0),
             IMP.algebra.Vector3D(-1,0,0), IMP.algebra.Vector3D(0,-1,0)]

        ps= IMP.Particles()
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
        w= IMP.display.BildWriter("derivs")
        w.set_file_name(self.get_tmp_file_name("deriv.bild"))
        for i in range(0,4):
            w.add_geometry(IMP.display.XYZDerivativeGeometry(IMP.core.XYZ(ps[i])))
        del w

        rbd= IMP.core.RigidBody.setup_particle(p, IMP.core.XYZs(ps))
        IMP.set_log_level(IMP.TERSE)
        print "eval"
        m.evaluate(True)
        w= IMP.display.BildWriter("qderivs")
        w.set_file_name(self.get_tmp_file_name("qderiv.bild"))
        #oge= display.XYZRGeometryExtractor(FloatKey("hi"))
        for i in range(0,4):
            gs = IMP.display.RigidBodyDerivativeGeometry(rbd)
            w.add_geometry(gs)
            print gs
        del w

if __name__ == '__main__':
    unittest.main()
