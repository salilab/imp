import unittest
import IMP, IMP.test

class OneSingle(IMP.SingletonScore):
    def __init__(self):
        IMP.SingletonScore.__init__(self)
    def evaluate(self, pa, da):
        return 1
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def show(self, t):
        print "One Singleton"

class Linear(IMP.UnaryFunction):
    def __init__(self):
        IMP.UnaryFunction.__init__(self)
    def evaluate(self, feat):
        return feat
    def evaluate_deriv(self, feat):
        return feat, 1.0
    def show(self, *args):
        print "identity"

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
        s= IMP.SingletonListRestraint(os, m.get_particles())
        m.add_restraint(s)
        score= m.evaluate(False)
        self.assertEqual(score, 10, "Wrong score")

    def test_ss(self):
        """Test the distanceto score"""
        m= IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(0)
        d.set_y(1)
        d.set_z(1)
        d.set_coordinates_are_optimized(True)
        v= IMP.Vector3D(3,1,5)
        l= Linear()
        s= IMP.DistanceToSingletonScore(l, v)
        r= IMP.SingletonListRestraint(s, m.get_particles())
        m.add_restraint(r)
        e= m.evaluate(False)
        self.assertEqual(e,5, "Wrong distance in score")

    def test_ss2(self):
        """Test the enclosing sphere """
        m= IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(100)
        d.set_y(1)
        d.set_z(1)
        d.set_coordinates_are_optimized(True)
        v= IMP.Vector3D(5,5,5)
        h= IMP.HarmonicUpperBound(10, 10)
        s= IMP.DistanceToSingletonScore(h, v)
        r= IMP.SingletonListRestraint(s, m.get_particles())
        m.add_restraint(r)
        e= m.evaluate(False)
        o= IMP.ConjugateGradients()
        o.set_model(m)
        o.optimize(100)
        d= IMP.XYZDecorator.cast(m.get_particle(IMP.ParticleIndex(0)))
        dist2 = (d.get_x()-5)**2+(d.get_y()-5)**2+(d.get_y()-5)**2
        print "Final"
        d.show()
        self.assert_(dist2 < 100.01, "Enclosing sphere not enclosing")

if __name__ == '__main__':
    unittest.main()
