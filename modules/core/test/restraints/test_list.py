import unittest
import IMP, IMP.test
import IMP.core
import IMP.algebra

class OneSingle(IMP.SingletonScore):
    def __init__(self):
        IMP.SingletonScore.__init__(self)
    def evaluate(self, pa, da):
        return 1
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def show(self, t):
        print "One Singleton"

class TestList(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)

    def test_it(self):
        """Test the list restraint"""
        m= IMP.Model()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
        os= OneSingle()
        s= IMP.core.SingletonListRestraint(os, m.get_particles())
        m.add_restraint(s)
        score= m.evaluate(False)
        self.assertEqual(score, 10, "Wrong score")

    def test_interacting_particles(self):
        """Test SingletonListRestraint::get_interacting_particles()"""
        m= IMP.Model()
        ps= []
        for i in range(0,10):
            p= IMP.Particle(m)
            ps.append(p)
        os= OneSingle()
        s= IMP.core.SingletonListRestraint(os, m.get_particles())
        m.add_restraint(s)
        ipar = s.get_interacting_particles()
        # Should return a one-element set for each particle:
        self.assertEqual(len(ipar), 10)
        for n, val in enumerate(ipar):
            self.assertEqual(len(val), 1)
            self.assertEqual(val[0], ps[n])

    def test_ss(self):
        """Test the distanceto score"""
        m= IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        d=IMP.core.XYZDecorator.create(p)
        d.set_x(0)
        d.set_y(1)
        d.set_z(1)
        d.set_coordinates_are_optimized(True)
        v= IMP.algebra.Vector3D(3,1,5)
        l= IMP.core.Linear(0, 1)
        s= IMP.core.DistanceToSingletonScore(l, v)
        r= IMP.core.SingletonListRestraint(s, m.get_particles())
        m.add_restraint(r)
        e= m.evaluate(False)
        self.assertEqual(e,5, "Wrong distance in score")

    def test_ss2(self):
        """Test the enclosing sphere """
        m= IMP.Model()
        p= IMP.Particle(m)
        d=IMP.core.XYZDecorator.create(p)
        d.set_x(100)
        d.set_y(1)
        d.set_z(1)
        d.set_coordinates_are_optimized(True)
        v= IMP.algebra.Vector3D(5,5,5)
        h= IMP.core.HarmonicUpperBound(10, 10)
        s= IMP.core.DistanceToSingletonScore(h, v)
        r= IMP.core.SingletonListRestraint(s, m.get_particles())
        m.add_restraint(r)
        e= m.evaluate(False)
        o= IMP.core.ConjugateGradients()
        o.set_model(m)
        o.optimize(100)
        d= IMP.core.XYZDecorator.cast(p)
        dist2 = (d.get_x()-5)**2+(d.get_y()-5)**2+(d.get_y()-5)**2
        print "Final"
        d.show()
        self.assert_(dist2 < 100.01, "Enclosing sphere not enclosing")

if __name__ == '__main__':
    unittest.main()
