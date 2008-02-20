import unittest
import IMP, IMP.test

class OneSingle(IMP.SingletonScore):
    def __init__(self):
        IMP.SingletonScore.__init__(self)
    def evaluate(self, pa, da):
        return 1
    def last_modified_by(self):
        return "Me"
    def version(self):
        return "0.5"
    def show(self, t):
        print "One Singleton"

class Linear(IMP.UnaryFunction):
    def __init__(self):
        IMP.UnaryFunction.__init__(self)
    def __call__(self, *args):
        return args[0]
    def show(self, *args):
        print "identity"

class TestList(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.VERBOSE)

    def test_it(self):
        """Test the list restraint"""
        m= IMP.Model()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
        os= OneSingle()
        s= IMP.SingletonListRestraint(m.get_particles(), os)
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
        s= IMP.DistanceToSingletonScore(v, l)
        r= IMP.SingletonListRestraint(m.get_particles(), s)
        m.add_restraint(r)
        e= m.evaluate(False)
        self.assertEqual(e,5, "Wrong distance in score")


if __name__ == '__main__':
    unittest.main()
