import unittest
import IMP, IMP.test
import os.path

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.TERSE)

    def _testit(self, rk, r,g,b, pref):
        """Test logging to a VRML file"""
        m= IMP.Model()
        o= IMP.SteepestDescent()
        o.set_model(m)
        nm="/tmp/"+pref+"vrmltest%03d.vrml"
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
        a= IMP.VRMLLogOptimizerState(nm, IMP.Particles([p0,p1]))
        a.set_radius(rk)
        a.set_color(r, g, b)
        o.add_optimizer_state(a)
        a.update()

        self.assert_(os.path.isfile("/tmp/"+pref+"vrmltest000.vrml"))
        os.remove("/tmp/"+pref+"vrmltest000.vrml")
    def test_1(self):
        """Testing the VRML log"""
        self._testit(IMP.FloatKey("radius"),
                     IMP.FloatKey("red"),
                     IMP.FloatKey("green"),
                     IMP.FloatKey("blue"), "test1")

    def test_2(self):
        """Testing the VRML log with new attribute names"""
        self._testit(IMP.FloatKey("another_radius"),
                     IMP.FloatKey("red5"),
                     IMP.FloatKey("green5"),
                     IMP.FloatKey("blue5"),
                     "test1")
    def test_skip(self):
        """Test skipping steps in the VRML log"""
        m= IMP.Model()
        o= IMP.SteepestDescent()
        o.set_model(m)
        nm="/tmp/"+"skip"+"vrmltest%03d.vrml"
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
        a= IMP.VRMLLogOptimizerState(nm, IMP.Particles([p0,p1]))
        a.set_skip_steps(20) # kind of a hack
        r= IMP.DistanceRestraint(p0, p1, IMP.Harmonic(0,10));
        m.add_restraint(r);
        o.add_optimizer_state(a)
        o.optimize(11)

        self.assert_(os.path.isfile("/tmp/"+"skip"+"vrmltest000.vrml"))
        self.assert_(os.path.isfile("/tmp/"+"skip"+"vrmltest001.vrml"))
        self.assert_(not os.path.isfile("/tmp/"+"skip"+"vrmltest002.vrml"))
        os.remove("/tmp/"+"skip"+"vrmltest000.vrml")
        os.remove("/tmp/"+"skip"+"vrmltest001.vrml")
        if (os.path.isfile("/tmp/"+"skip"+"vrmltest002.vrml")):
            os.remove("/tmp/"+"skip"+"vrmltest002.vrml")
if __name__ == '__main__':
    unittest.main()
