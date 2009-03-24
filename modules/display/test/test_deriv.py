import IMP
import IMP.algebra
import IMP.display
import IMP.core
from IMP import *


import unittest
import IMP, IMP.test
import IMP.core
import IMP.display
import os.path

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def test_derivs(self):
        """Testing execution of derivative display support"""
        # note that there are no actual checks here at this point
        algebra=IMP.algebra
        display= IMP.display
        m= Model()
        pts=[algebra.Vector3D(1,0,0), algebra.Vector3D(0,1,0),
             algebra.Vector3D(-1,0,0), algebra.Vector3D(0,-1,0)]

        ps= Particles()
        for i in range(0,4):
            p= Particle(m)
            d= core.XYZDecorator.create(p, pts[i])
            ps.append(p)
        p= Particle(m)
        d= core.XYZDecorator.create(p)
        hd= core.HierarchyDecorator.create(p, ps)

        for i in range(0,4):
            u= core.Harmonic(0,1)
            s= core.DistanceToSingletonScore(u, pts[(i+1)%4])
            r= core.SingletonRestraint(s, ps[i])
            m.add_restraint(r)

        m.evaluate(True)
        w= display.BildWriter()
        w.set_file_name(self.get_tmp_file_name("deriv.bild"))
        for i in range(0,4):
            w.add_geometry(display.XYZDerivativeGeometry(core.XYZDecorator(ps[i])))
        w.set_file_name("")

        ss= core.create_rigid_body(p, ps)
        rbd= IMP.core.RigidBodyDecorator(p)
        set_log_level(TERSE)
        print "eval"
        m.evaluate(True)
        w= display.BildWriter()
        w.set_file_name(self.get_tmp_file_name("qderiv.bild"))
        #oge= display.XYZRGeometryExtractor(FloatKey("hi"))
        for i in range(0,4):
            gs=display.RigidBodyDerivativeGeometry(rbd)
            w.add_geometry(gs)
            print gs
        w.set_file_name("")
if __name__ == '__main__':
    unittest.main()
