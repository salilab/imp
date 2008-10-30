import unittest
import IMP.utils
import IMP.test, IMP
import IMP.core

class DistanceTests(IMP.test.TestCase):
    """Test the symmetry restraint"""
    def test_symmetry(self):
        """Test the transform pair score basics"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.core.XYZDecorator.create(p0)
        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.core.XYZDecorator.create(p1)
        tps= IMP.core.TransformedDistancePairScore(IMP.core.Harmonic(0,1))
        tps.set_translation(IMP.Vector3D(0,1,0))
        tps.set_rotation(1, 0, 0,
                         0, 1, 0,
                         0, 0, 1)
        d0.set_coordinates(IMP.Vector3D(2,3,4))
        d1.set_coordinates(IMP.Vector3D(2,2,4))
        self.assertEqual(tps.evaluate(p0, p1, None), 0)
        self.assert_(tps.evaluate(p1, p0, None) != 0)
        tps.set_center(IMP.Vector3D(10,13,4))
        self.assertEqual(tps.evaluate(p0, p1, None), 0)
        self.assert_(tps.evaluate(p1, p0, None) != 0)
        tps.set_center(IMP.Vector3D(0,0,0))
        print "test rotation"
        tps.set_rotation(0, 0,-1,
                         0, 1, 0,
                         1, 0, 0)
        d1.set_coordinates(IMP.Vector3D(4, 2, -2))
        self.assertEqual(tps.evaluate(p0, p1, None), 0)
        self.assert_(tps.evaluate(p1, p0, None) != 0)
        tps.set_translation(IMP.Vector3D(0,0,0))
        tps.set_rotation(0,-1, 0,
                         1, 0, 0,
                         0, 0, 1)
        d0.set_coordinates(IMP.Vector3D(0,1,0))
        d1.set_coordinates(IMP.Vector3D(1,1,0))
        # clear derivs
        print "test derivs"
        m.evaluate(True)
        tps.evaluate(p0, p1, IMP.DerivativeAccumulator(1))
        print d0.get_coordinate_derivative(0)
        print d0.get_coordinate_derivative(1)
        print d0.get_coordinate_derivative(2)
        print d1.get_coordinate_derivative(0)
        print d1.get_coordinate_derivative(1)
        print d1.get_coordinate_derivative(2)
        self.assert_(d0.get_coordinate_derivative(0) > 0)
        self.assert_(d0.get_coordinate_derivative(1) == 0)
        self.assert_(d0.get_coordinate_derivative(2) == 0)
        self.assert_(d1.get_coordinate_derivative(1) > 0)
        self.assert_(d1.get_coordinate_derivative(0) == 0)
        self.assert_(d1.get_coordinate_derivative(2) == 0)
    def test_symmetry2(self):
        """Test the transform pair score optimization"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p0= IMP.Particle()
        m.add_particle(p0)
        d0= IMP.core.XYZDecorator.create(p0)
        p1= IMP.Particle()
        m.add_particle(p1)
        d1= IMP.core.XYZDecorator.create(p1)
        d0.set_coordinates(IMP.Vector3D(2,3,4))
        d1.set_coordinates(IMP.Vector3D(20,20,40))
        d0.set_coordinates_are_optimized(True)
        d1.set_coordinates_are_optimized(True)
        tps= IMP.core.TransformedDistancePairScore(IMP.core.Harmonic(0,1))
        tps.set_translation(IMP.Vector3D(0,1,0))
        tps.set_rotation(1, 0, 0,
                         0, 0,-1,
                         0, 1, 0)
        pr= IMP.core.PairListRestraint(tps)
        pr.add_particle_pair(IMP.ParticlePair(p0, p1))
        m.add_restraint(pr)
        cg= IMP.core.ConjugateGradients()
        cg.set_model(m)
        cg.optimize(100)
        d0.show()
        d1.show()
        vt= IMP.Vector3D(d1.get_coordinates()*IMP.Vector3D(1,0,0)+0,
                         d1.get_coordinates()*IMP.Vector3D(0,0,-1)+1,
                         d1.get_coordinates()*IMP.Vector3D(0,1,0)+0)
        print "trans"
        print str(vt[0]) + " " + str(vt[1])+" " + str(vt[2])
        self.assertEqual(vt[0], d0.get_coordinate(0))
        self.assertEqual(vt[1], d0.get_coordinate(1))
        self.assertEqual(vt[2], d0.get_coordinate(2))

if __name__ == '__main__':
    unittest.main()
