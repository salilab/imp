import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.utils
import IMP.core
import IMP.algebra
import math

class TunnelTest(IMP.test.TestCase):
    """Tests for tunnel scores"""

    def _test_score(self):
        """Test derivatives and score of tunnel score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        rk= IMP.FloatKey("radiusk")
        d= IMP.core.XYZRDecorator.create(p, rk)
        d.set_radius(1)
        f= IMP.core.HarmonicLowerBound(0, 1)
        tss= IMP.misc.TunnelSingletonScore(f, rk)
        tss.set_height(10)
        tss.set_radius(5)
        tss.set_center(IMP.algebra.Vector3D(10,10,10))
        tss.set_coordinate(2)
        l= IMP.core.ListSingletonContainer()
        sl= IMP.core.SingletonsRestraint(tss, l)
        l.add_particle(p)
        m.add_restraint(sl)
        d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        print "test first"
        m.evaluate(True)
        print "###########test second"
        d.set_coordinates(IMP.algebra.Vector3D(10,10,10))
        self.assertEqual(m.evaluate(True), 0)

        print "############Test left"
        d.set_coordinates(IMP.algebra.Vector3D(4, 10, 10))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(2), 0)
        self.assertEqual(d.get_coordinate_derivative(1), 0)
        print d.get_coordinate_derivative(0)
        self.assert_(d.get_coordinate_derivative(0) < 0)

        print "##############Test above"
        d.set_coordinates(IMP.algebra.Vector3D(10, 4, 10))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(2), 0)
        self.assertEqual(d.get_coordinate_derivative(0), 0)
        print d.get_coordinate_derivative(1)
        self.assert_(d.get_coordinate_derivative(1) < 0)

        print "############Test bottom"
        d.set_coordinates(IMP.algebra.Vector3D(30, 30, 3))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(0), 0)
        self.assertEqual(d.get_coordinate_derivative(1), 0)
        self.assert_(d.get_coordinate_derivative(2) > 0)

        print "#############Test top"
        d.set_coordinates(IMP.algebra.Vector3D(30, 30, 17))
        self.assert_(m.evaluate(True)>0)
        self.assertEqual(d.get_coordinate_derivative(0), 0)
        self.assertEqual(d.get_coordinate_derivative(1), 0)
        self.assert_(d.get_coordinate_derivative(2) < 0)

    def test_optimize(self):
        """Test optimize tunnel score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.Particle(m)
        rk= IMP.FloatKey("radiusk")
        d= IMP.core.XYZRDecorator.create(p, rk)
        d.set_radius(1)
        f= IMP.core.HarmonicLowerBound(0, 1)
        tss= IMP.misc.TunnelSingletonScore(f, rk)
        tss.set_height(10)
        tss.set_radius(5)
        tss.set_center(IMP.algebra.Vector3D(10,10,10))
        tss.set_coordinate(2)
        l= IMP.core.ListSingletonContainer()
        sl= IMP.core.SingletonsRestraint(tss, l)
        l.add_particle(p)
        m.add_restraint(sl)
        d.set_coordinates(IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(-20, -20, -20),
                                                           IMP.algebra.Vector3D(50, 50, 50)))
        d.set_coordinates_are_optimized(True)
        cg= IMP.core.ConjugateGradients()
        cg.set_model(m)
        d.show()
        m.evaluate(True)
        IMP.set_log_level(IMP.SILENT)
        cg.optimize(1000)
        d.show()
        self.assert_(m.evaluate(False) < .001)

if __name__ == '__main__':
    unittest.main()
