import unittest
import IMP, IMP.test
import IMP.core
import IMP.display
import os.path

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def _testit(self, pref, suf, writer):
        m= IMP.Model()
        o= IMP.core.SteepestDescent()
        o.set_model(m)
        rk= IMP.FloatKey("my radius_key")
        nm = pref + "test%03d."+suf
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.core.XYZRDecorator.create(p0, rk)
        d0.set_radius(1.5)
        d0.set_x(0)
        d0.set_y(0)
        d0.set_z(0)

        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.core.XYZRDecorator.create(p1)
        d1.set_x(1)
        d1.set_y(1)
        d1.set_z(1)
        d1.set_radius(1)
        IMP.set_log_level(IMP.MEMORY)
        a= IMP.display.LogOptimizerState(writer, nm)
        g= IMP.display.XYZRGeometry(d0)
        #ge= IMP.display.XYZRGeometryExtractor(rk)
        ps= IMP.core.ListSingletonContainer()
        ps.add_particle(p1)
        #a.add_geometry(ge, ps)
        a.add_geometry(g)
        o.add_optimizer_state(a)
        a.update()

        print "name is "+(nm%0)
        os.remove(nm%0)

    def test_1(self):
        """Testing the VRML log"""
        self._testit("testvrml", "vrml", IMP.display.VRMLWriter())

    def test_2(self):
        """Testing the CMM log"""
        self._testit("testcmm", "cmm", IMP.display.CMMWriter())


    def test_3(self):
        """Testing the Bild log"""
        self._testit("testbild", "bild", IMP.display.BildWriter())

if __name__ == '__main__':
    unittest.main()
