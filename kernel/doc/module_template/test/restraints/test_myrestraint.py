import unittest
import IMP
import IMP.test
import IMP.modulename
import IMP.core


class GenericTest(IMP.test.TestCase):
    """Some generic test"""

    def test_myrestraint(self):
        """Checking MyRestraint"""
        m = IMP.Model()
        p0= IMP.Particle(m)
        d0= IMP.core.XYZDecorator.create(p0)
        p1= IMP.Particle(m)
        d1= IMP.core.XYZDecorator.create(p1)
        dps= IMP.core.DistancePairScore(IMP.core.Linear(0,1))
        r= IMP.modulename.MyRestraint(dps, p0, p1)
        d0.set_coordinates(IMP.Vector3D(0,0,0))
        d1.set_coordinates(IMP.Vector3D(0,0,1))
        print m
        print r
        print p0
        print p1
        print dps
        m.add_restraint(r)
        self.assertInTolerance(m.evaluate(False), 1, .01)

if __name__ == '__main__':
    unittest.main()
