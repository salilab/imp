import unittest
import IMP
import IMP.test
import IMP.utils
import math

class TunnelTest(IMP.test.TestCase):
    """Tests for tunnel restraints"""

    def test_score(self):
        """Test derivatives and score of tunnel restraint"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        d= IMP.XYZDecorator.create(p)
        rk= IMP.FloatKey("radiusk")
        p.add_attribute(rk, 1, False)
        f= IMP.HarmonicLowerBound(0, 1)
        tr= IMP.TunnelRestraint(f, rk)
        tr.set_height(10)
        tr.set_radius(5)
        tr.set_center(IMP.Vector3D(10,10,10))
        tr.add_particle(p)
        tr.set_coordinate(2)
        m.add_restraint(tr)
        m.evaluate(True)
        d.set_coordinates(IMP.Vector3D(10,10,10))
        self.assertEqual(m.evaluate(True), 0)

        print "Test left"
        d.set_coordinates(IMP.Vector3D(4, 10, 10))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(2), 0)
        self.assertEqual(d.get_coordinate_derivative(1), 0)
        print d.get_coordinate_derivative(0)
        self.assert_(d.get_coordinate_derivative(0) > 0)

        print "Test above"
        d.set_coordinates(IMP.Vector3D(10, 4, 10))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(2), 0)
        self.assertEqual(d.get_coordinate_derivative(0), 0)
        print d.get_coordinate_derivative(1)
        self.assert_(d.get_coordinate_derivative(1) > 0)

        print "Test bottom"
        d.set_coordinates(IMP.Vector3D(30, 30, 3))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(0), 0)
        self.assertEqual(d.get_coordinate_derivative(1), 0)
        self.assert_(d.get_coordinate_derivative(2) < 0)

        print "Test top"
        d.set_coordinates(IMP.Vector3D(30, 30, 17))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(0), 0)
        self.assertEqual(d.get_coordinate_derivative(1), 0)
        self.assert_(d.get_coordinate_derivative(2) > 0)

if __name__ == '__main__':
    unittest.main()
