import IMP
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    def make_system(self):
        m = IMP.Model()
        p1 = m.add_particle("p1")
        IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1., 2., 3.))
        ss = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), IMP.algebra.Vector3D(0., 0., 0.))
        r = IMP.core.SingletonRestraint(m, ss, p1)
        return m, p1, ss, r

    def test_pickle(self):
        """Test (un-)pickle of SingletonRestraint"""
        m, p1, ss, r = self.make_system()
        r.set_name("foo")
        self.assertAlmostEqual(r.evaluate(False), 7.0, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 7.0, delta=1e-4)
        self.assertEqual(newr.get_name(), "foo")

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SingletonRestraint via polymorphic pointer"""
        m, p1, ss, r = self.make_system()
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        self.assertAlmostEqual(sf.evaluate(False), 7.0, delta=1e-4)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        self.assertAlmostEqual(newsf.evaluate(False), 7.0, delta=1e-4)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
