from __future__ import print_function
import IMP
import IMP.test
import IMP.pmi

class Tests(IMP.test.TestCase):

    def test_symmetric(self):
        """Test Symmetric decorator"""
        m = IMP.Model()
        pi = m.add_particle("p1")
        self.assertFalse(IMP.pmi.Symmetric.get_is_setup(m, pi))
        d = IMP.pmi.Symmetric.setup_particle(m, pi, 1)
        self.assertTrue(IMP.pmi.Symmetric.get_is_setup(m, pi))
        self.assertAlmostEqual(d.get_symmetric(), 1, delta=0.01)
        d.set_symmetric(0)
        d.show()
        IMP.check_particle(m, pi)
        self.assertAlmostEqual(d.get_symmetric(), 0, delta=0.01)

        # Negative symmetric is not allowed
        d.set_symmetric(-1)
        self.assertRaises(ValueError, IMP.check_particle, m, pi)


if __name__ == '__main__':
    IMP.test.main()
