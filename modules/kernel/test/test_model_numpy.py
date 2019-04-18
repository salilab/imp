from __future__ import print_function
import IMP
import IMP.core
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

    def test_get_spheres_numpy(self):
        """Test _get_spheres_numpy method"""
        m = IMP.Model("score state show")
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        p3 = IMP.Particle(m)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        d1.set_radius(4)

        d2 = IMP.core.XYZR.setup_particle(p2)
        d2.set_coordinates(IMP.algebra.Vector3D(5,6,7))
        d2.set_radius(8)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m._get_spheres_numpy()
            self.assertAlmostEqual(n[0][0], 4.0, delta=1e-4)
            self.assertAlmostEqual(n[0][1], 1.0, delta=1e-4)
            self.assertAlmostEqual(n[0][2], 2.0, delta=1e-4)
            self.assertAlmostEqual(n[0][3], 3.0, delta=1e-4)

            self.assertAlmostEqual(n[1][0], 8.0, delta=1e-4)
            self.assertAlmostEqual(n[1][1], 5.0, delta=1e-4)
            self.assertAlmostEqual(n[1][2], 6.0, delta=1e-4)
            self.assertAlmostEqual(n[1][3], 7.0, delta=1e-4)
            n[0][0] = 42.0
            self.assertAlmostEqual(d1.get_radius(), 42.0, delta=1e-6)
            n[1][1] = 24.0
            self.assertAlmostEqual(d2.get_coordinates()[0], 24.0, delta=1e-6)
        else:
            self.assertRaises(NotImplementedError, m._get_spheres_numpy)

    def test_get_sphere_derivatives_numpy(self):
        """Test _get_sphere_derivatives_numpy method"""
        m = IMP.Model("score state show")
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        p3 = IMP.Particle(m)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m._get_sphere_derivatives_numpy()
            n[0][1] = 42.0
            n[1][2] = 24.0
            self.assertAlmostEqual(d1.get_derivatives()[0], 42.0, delta=1e-6)
            self.assertAlmostEqual(d2.get_derivatives()[1], 24.0, delta=1e-6)
        else:
            self.assertRaises(NotImplementedError,
                              m._get_sphere_derivatives_numpy)


if __name__ == '__main__':
    IMP.test.main()
