import unittest
import IMP, IMP.test
import IMP.core
import IMP.algebra
import os.path

class TestBL(IMP.test.TestCase):
    def test_it(self):
        """Test diameter helper function"""
        diameter=1
        m= IMP.Model()
        ps= IMP.core.create_xyzr_particles(m, 20, 1.0)
        for p in ps:
            IMP.core.XYZDecorator(p).set_coordinates_are_optimized(True)
        r=IMP.core.DiameterRestraint(IMP.core.Harmonic(0,1), IMP.core.ListSingletonContainer(ps), diameter)
        m.add_restraint(r)
        # Set up optimizer
        o= IMP.core.ConjugateGradients()
        o.set_model(m)

        o.optimize(100)

        for i in range(0, len(ps)):
            for j in range(0, len(ps)):
                p0= ps[i]
                p1= ps[j]
                print IMP.core.distance(IMP.core.XYZDecorator(p0),
                                        IMP.core.XYZDecorator(p1))
                self.assert_(IMP.core.distance(IMP.core.XYZDecorator(p0),
                                               IMP.core.XYZDecorator(p1))
                             < diameter*1.05)





if __name__ == '__main__':
    unittest.main()
