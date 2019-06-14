from __future__ import print_function
import IMP
import IMP.test
import IMP.pmi

class Tests(IMP.test.TestCase):

    def test_uncertainty(self):
        """Test Uncertainty decorator"""
        m = IMP.Model()
        pi = m.add_particle("p1")
        self.assertFalse(IMP.pmi.Uncertainty.get_is_setup(m, pi))
        d = IMP.pmi.Uncertainty.setup_particle(m, pi, 0.8)
        self.assertTrue(IMP.pmi.Uncertainty.get_is_setup(m, pi))
        self.assertAlmostEqual(d.get_uncertainty(), 0.8, delta=0.01)
        d.set_uncertainty(0.9)
        d.show()
        IMP.check_particle(m, pi)
        self.assertAlmostEqual(d.get_uncertainty(), 0.9, delta=0.01)

        # Negative uncertainty is not allowed
        d.set_uncertainty(-0.9)
        self.assertRaises(ValueError, IMP.check_particle, m, pi)


if __name__ == '__main__':
    IMP.test.main()
