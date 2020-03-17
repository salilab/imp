import IMP
import IMP.isd
import IMP.test


class Tests(IMP.test.TestCase):
    def test_uniform_prior(self):
        """Test UniformPrior"""
        m = IMP.Model()
        sigma = IMP.isd.Scale.setup_particle(IMP.Particle(m), 1.0)
        p = IMP.isd.UniformPrior(m, sigma, 1000, 100, 0.01)
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


if __name__ == '__main__':
    IMP.test.main()
