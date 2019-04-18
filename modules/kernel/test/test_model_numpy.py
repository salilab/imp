from __future__ import print_function
import IMP
import IMP.test

class Tests(IMP.test.TestCase):

    def test_get_derivatives_numpy(self):
        """Test _get_derivatives_numpy method"""
        m = IMP.Model("score state show")
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)

        k = IMP.FloatKey("myf")
        p1.add_attribute(k, 1.0)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m._get_derivatives_numpy(k)
            self.assertAlmostEqual(n[0], 0.0, delta=1e-6)
            n[0] = 42.0
            self.assertAlmostEqual(p1.get_derivative(k), 42.0, delta=1e-6)
        else:
            self.assertRaises(NotImplementedError, m._get_derivatives_numpy, k)


if __name__ == '__main__':
    IMP.test.main()
