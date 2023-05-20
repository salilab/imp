import IMP
import IMP.isd
import IMP.test
import pickle


def _make_test_restraint():
    m = IMP.Model()
    sigmap = IMP.Particle(m)
    sigma = IMP.isd.Scale.setup_particle(sigmap, 1.0)
    p = IMP.isd.UniformPrior(m, sigma, 1000, 100, 0.01)
    return m, sigma, p

class Tests(IMP.test.TestCase):
    def test_uniform_prior(self):
        """Test UniformPrior"""
        m, sigma, p = _make_test_restraint()
        self.assertAlmostEqual(p.get_probability(), 1.0, delta=1e-6)
        self.assertAlmostEqual(p.evaluate(False), 0.0, delta=1e-6)
        s = p.get_static_info()
        s.set_was_used(True)
        self.assertEqual(s.get_number_of_filename(), 0)
        self.assertEqual(s.get_number_of_string(), 1)
        self.assertEqual(s.get_string_key(0), "type")
        self.assertEqual(s.get_string_value(0), "IMP.isd.UniformPrior")

        self.assertEqual(s.get_number_of_int(), 0)
        self.assertEqual(s.get_number_of_float(), 3)
        self.assertEqual(s.get_float_key(0), "force constant")
        self.assertAlmostEqual(s.get_float_value(0), 1000, delta=1e-6)
        self.assertEqual(s.get_float_key(1), "lower bound")
        self.assertAlmostEqual(s.get_float_value(1), 0.01, delta=1e-6)
        self.assertEqual(s.get_float_key(2), "upper bound")
        self.assertAlmostEqual(s.get_float_value(2), 100, delta=1e-6)

        s = p.get_dynamic_info()
        s.set_was_used(True)
        self.assertEqual(s.get_number_of_particle_indexes(), 1)
        self.assertEqual(s.get_particle_indexes_key(0), "particle")
        self.assertEqual(s.get_particle_indexes_value(0),
                         [sigma.get_particle_index()])

    def test_serialize(self):
        """Test (un-)serialize of UniformPrior"""
        m, sigma, r = _make_test_restraint()
        sigma.set_scale(105.)
        r.set_name("foo")
        self.assertAlmostEqual(r.evaluate(True), 12500.0, delta=0.001)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(True), 12500.0, delta=0.001)

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of UniformPrior via polymorphic pointer"""
        m, sigma, r = _make_test_restraint()
        sigma.set_scale(105.)
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        self.assertAlmostEqual(sf.evaluate(True), 12500.0, delta=0.001)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(True), 12500.0, delta=0.001)


if __name__ == '__main__':
    IMP.test.main()
