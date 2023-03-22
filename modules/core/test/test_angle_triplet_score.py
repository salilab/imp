import IMP
import IMP.test
import IMP.core
import pickle


def _make_model():
    m = IMP.Model()
    p1 = IMP.Particle(m)
    p2 = IMP.Particle(m)
    p3 = IMP.Particle(m)
    d1 = IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(0,0,0))
    d2 = IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(0,1,0))
    d3 = IMP.core.XYZ.setup_particle(p3, IMP.algebra.Vector3D(1,0,0))
    return m, p1, p2, p3, d1, d2, d3


class Tests(IMP.test.TestCase):

    def test_pickle_cosine(self):
        """Test (un-)pickle of AngleTripletScore with Cosine"""
        m, p1, p2, p3, d1, d2, d3 = _make_model()

        uf = IMP.core.Cosine(10.0, 2, 0.0)
        ats = IMP.core.AngleTripletScore(uf)
        ats.set_name('foo')
        self.assertAlmostEqual(ats.evaluate_index(m, [p1, p2, p3], None),
                               10.0, delta=1e-2)

        dump = pickle.dumps(ats)
        newats = pickle.loads(dump)
        self.assertEqual(ats.get_name(), 'foo')
        self.assertAlmostEqual(newats.evaluate_index(m, [p1, p2, p3], None),
                               10.0, delta=1e-2)

    def test_pickle_linear(self):
        """Test (un-)pickle of AngleTripletScore with Linear"""
        m, p1, p2, p3, d1, d2, d3 = _make_model()

        uf = IMP.core.Linear(-1.0, 2.0)
        ats = IMP.core.AngleTripletScore(uf)
        ats.set_name('foo')
        self.assertAlmostEqual(ats.evaluate_index(m, [p1, p2, p3], None),
                               3.5708, delta=1e-2)

        dump = pickle.dumps(ats)
        newats = pickle.loads(dump)
        self.assertEqual(ats.get_name(), 'foo')
        self.assertAlmostEqual(newats.evaluate_index(m, [p1, p2, p3], None),
                               3.5708, delta=1e-2)

    def test_pickle_harmonic(self):
        """Test (un-)pickle of AngleTripletScore with Harmonic"""
        m, p1, p2, p3, d1, d2, d3 = _make_model()

        uf = IMP.core.Harmonic(1.0, 10.0)
        ats = IMP.core.AngleTripletScore(uf)
        ats.set_name('foo')
        self.assertAlmostEqual(ats.evaluate_index(m, [p1, p2, p3], None),
                               0.2303, delta=1e-2)

        dump = pickle.dumps(ats)
        newats = pickle.loads(dump)
        self.assertEqual(ats.get_name(), 'foo')
        self.assertAlmostEqual(newats.evaluate_index(m, [p1, p2, p3], None),
                               0.2303, delta=1e-2)

    def test_pickle_unregistered(self):
        """Test (un-)pickle of AngleTripletScore with unregistered class"""
        m, p1, p2, p3, d1, d2, d3 = _make_model()

        uf = IMP.core.OpenCubicSpline([1.,2.,3.], 0., 1.)
        ats = IMP.core.AngleTripletScore(uf)
        # OpenCubicSpline is (currently) unregistered, so this should fail
        # with a "trying to save an unregistered polymorphic type" error
        self.assertRaises(TypeError, pickle.dumps, ats)


if __name__ == '__main__':
    IMP.test.main()
