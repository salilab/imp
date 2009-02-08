import unittest
import IMP, IMP.test
import IMP.core
import os.path

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def _testit(self, rk, r,g,b, name,pref):
        """Test logging to a CMM file"""
        m= IMP.Model()
        o= IMP.core.SteepestDescent()
        o.set_model(m)
        nm = pref + "cmmtest%03d.cmm"
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.core.XYZDecorator.create(p0)
        p0.add_attribute(rk, 1.5, False)
        d0.set_x(0)
        d0.set_y(0)
        d0.set_z(0)

        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.core.XYZDecorator.create(p1)
        p1.add_attribute(r, 255)
        p1.add_attribute(g, 0)
        p1.add_attribute(b, 0)
        p1.add_attribute(name, "p1")
        d1.set_x(1)
        d1.set_y(1)
        d1.set_z(1)
        a= IMP.core.CMMLogOptimizerState(nm, IMP.Particles([p0,p1]))
        a.set_radius(rk)
        a.set_color(r, g, b)
        a.set_name(name)
        o.add_optimizer_state(a)
        a.update()

        os.remove(pref + "cmmtest000.cmm")

    def test_1(self):
        """Testing the CMM log"""
        self._testit(IMP.FloatKey("radius"),
                     IMP.IntKey("red"),
                     IMP.IntKey("green"),
                     IMP.IntKey("blue"), IMP.StringKey("name"),"test1")

    def test_2(self):
        """Testing the CMM log with new attribute names"""
        self._testit(IMP.FloatKey("another_radius"),
                     IMP.IntKey("red5"),
                     IMP.IntKey("green5"),
                     IMP.IntKey("blue5"),IMP.StringKey("name5"),
                     "test1")
    def test_skip(self):
        """Test skipping steps in the CMM log"""
        m= IMP.Model()
        o= IMP.core.SteepestDescent()
        o.set_model(m)
        nm = "skip" + "cmmtest%03d.cmm"
        # Possible clean up from any previous failed runs:
        try:
            os.remove("skip" + "cmmtest002.cmm")
        except OSError:
            pass
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.core.XYZDecorator.create(p0)
        p0.add_attribute(IMP.FloatKey("radius"), 1.5, False)
        d0.set_x(0)
        d0.set_y(0)
        d0.set_z(0)
        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.core.XYZDecorator.create(p1)
        d1.set_x(1)
        d1.set_y(1)
        d1.set_z(1)
        a= IMP.core.CMMLogOptimizerState(nm, IMP.Particles([p0,p1]))
        a.set_skip_steps(10)
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(0,10), p0, p1);
        m.add_restraint(r);
        o.add_optimizer_state(a)
        o.optimize(11)
        os.remove("skip" + "cmmtest000.cmm")
        os.remove("skip" + "cmmtest001.cmm")
        self.assert_(not os.path.isfile("skip" + "cmmtest002.cmm"))
        print "4"

if __name__ == '__main__':
    unittest.main()
