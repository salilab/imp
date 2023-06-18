import IMP
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of HarmonicUpperBound"""
        func = IMP.core.HarmonicUpperBound(1.0, 4.0)
        func.set_name('foo')
        self.assertAlmostEqual(func.evaluate(4.0), 18.0, delta=0.01)
        dump = pickle.dumps(func)
        del func
        f = pickle.loads(dump)
        self.assertEqual(f.get_name(), 'foo')
        self.assertAlmostEqual(f.evaluate(4.0), 18.0, delta=0.01)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of HarmonicUpperBound via polymorphic pointer"""
        m = IMP.Model()
        func = IMP.core.HarmonicUpperBound(1.0, 4.0)
        s = IMP.core.DistanceToSingletonScore(func, IMP.algebra.Vector3D(0,0,0))
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(4,0,0))
        self.assertAlmostEqual(s.evaluate_index(m, p, None), 18.0, delta=0.01)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertAlmostEqual(news.evaluate_index(m, p, None),
                               18.0, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
