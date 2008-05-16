import unittest
import IMP, IMP.test
import os.path

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def _testit(self, rk, r,g,b, pref):
        """Test logging to a CMM file"""
        m= IMP.Model()
        o= IMP.SteepestDescent()
        o.set_model(m)
        nm="/tmp/"+pref+"cmmtest%03d.cmm"
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.XYZDecorator.create(p0)
        p0.add_attribute(rk, 1.5, False)
        d0.set_x(0)
        d0.set_y(0)
        d0.set_z(0)

        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.XYZDecorator.create(p1)
        p1.add_attribute(r, 1.0, False)
        p1.add_attribute(g, 0.0, False)
        p1.add_attribute(b, 0.0, False)
        d1.set_x(1)
        d1.set_y(1)
        d1.set_z(1)
        a= IMP.CMMLogOptimizerState(nm, IMP.Particles([p0,p1]))
        a.set_radius(rk)
        a.set_color(r, g, b)
        o.add_optimizer_state(a)
        a.update()

        self.assert_(os.path.isfile("/tmp/"+pref+"cmmtest000.cmm"))
        os.remove("/tmp/"+pref+"cmmtest000.cmm")
    def test_1(self):
        """Testing the CMM log"""
        self._testit(IMP.FloatKey("radius"),
                     IMP.FloatKey("red"),
                     IMP.FloatKey("green"),
                     IMP.FloatKey("blue"), "test1")

    def test_2(self):
        """Testing the CMM log with new attribute names"""
        self._testit(IMP.FloatKey("another_radius"),
                     IMP.FloatKey("red5"),
                     IMP.FloatKey("green5"),
                     IMP.FloatKey("blue5"),
                     "test1")
    def test_skip(self):
        """Test skipping steps in the CMM log"""
        m= IMP.Model()
        o= IMP.SteepestDescent()
        o.set_model(m)
        nm="/tmp/"+"skip"+"cmmtest%03d.cmm"
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.XYZDecorator.create(p0)
        p0.add_attribute(IMP.FloatKey("radius"), 1.5, False)
        d0.set_x(0)
        d0.set_y(0)
        d0.set_z(0)
        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.XYZDecorator.create(p1)
        d1.set_x(1)
        d1.set_y(1)
        d1.set_z(1)
        a= IMP.CMMLogOptimizerState(nm, IMP.Particles([p0,p1]))
        a.set_skip_steps(20)
        r= IMP.DistanceRestraint(IMP.Harmonic(0,10), p0, p1);
        m.add_restraint(r);
        o.add_optimizer_state(a)
        o.optimize(11)
        self.assert_(os.path.isfile("/tmp/"+"skip"+"cmmtest000.cmm"))
        self.assert_(os.path.isfile("/tmp/"+"skip"+"cmmtest001.cmm"))
        self.assert_(not os.path.isfile("/tmp/"+"skip"+"cmmtest002.cmm"))
        os.remove("/tmp/"+"skip"+"cmmtest000.cmm")
        os.remove("/tmp/"+"skip"+"cmmtest001.cmm")
        if (os.path.isfile("/tmp/"+"skip"+"cmmtest002.cmm")):
            os.remove("/tmp/"+"skip"+"cmmtest002.cmm")
        print "4"
if __name__ == '__main__':
    unittest.main()
