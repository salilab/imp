import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle


class Tests(IMP.test.TestCase):

    def make_system(self):
        m = IMP.Model()
        p1 = m.add_particle("p1")
        IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1., 2., 3.))
        p2 = m.add_particle("p2")
        IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4., 5., 6.))
        ss = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), IMP.algebra.Vector3D(0., 0., 0.))
        lsc = IMP.container.ListSingletonContainer(m, [p1, p2])
        r = IMP.container.SingletonsRestraint(ss, lsc)
        return m, p1, p2, r

    def test_pickle(self):
        """Test (un-)pickle of SingletonsRestraint"""
        m, p1, p2, r = self.make_system()
        r.set_name("foo")
        self.assertAlmostEqual(r.evaluate(False), 45.5, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 45.5, delta=1e-4)
        self.assertEqual(newr.get_name(), "foo")

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SingletonsRestraint via polymorphic pointer"""
        m, p1, p2, r = self.make_system()
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        self.assertAlmostEqual(sf.evaluate(False), 45.5, delta=1e-4)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        self.assertAlmostEqual(newsf.evaluate(False), 45.5, delta=1e-4)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
