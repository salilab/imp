from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import sys

class Tests(IMP.test.TestCase):

    def test_get_derivatives_numpy(self):
        """Test _get_derivatives_numpy method"""
        m1 = IMP.Model("numpy get_derivs")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_derivs")
        p12 = IMP.Particle(m2)

        k = IMP.FloatKey("myf")
        p1.add_attribute(k, 1.0)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1._get_derivatives_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 1) # no derivatives for p2
            self.assertAlmostEqual(n[0], 0.0, delta=1e-6)
            n[0] = 42.0
            self.assertAlmostEqual(p1.get_derivative(k), 42.0, delta=1e-6)

            n = m2._get_derivatives_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no float key for this model
        else:
            self.assertRaises(NotImplementedError, m1._get_derivatives_numpy, k)

    def test_get_floats_numpy(self):
        """Test _get_floats_numpy method"""
        m1 = IMP.Model("numpy get_floats")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_floats")
        p12 = IMP.Particle(m2)

        k = IMP.FloatKey("myf")
        p1.add_attribute(k, 1.0)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1._get_floats_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 1) # no float attribute for p2
            self.assertAlmostEqual(n[0], 1.0, delta=1e-6)
            n[0] = 42.0
            self.assertAlmostEqual(p1.get_value(k), 42.0, delta=1e-6)

            n = m2._get_floats_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no float key for this model
        else:
            self.assertRaises(NotImplementedError, m1._get_floats_numpy, k)

    def test_get_ints_numpy(self):
        """Test _get_ints_numpy method"""
        m1 = IMP.Model("numpy get_ints")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_ints")
        p12 = IMP.Particle(m2)

        k = IMP.IntKey("myf")
        p1.add_attribute(k, 1)
        p2.add_attribute(k, 2)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1._get_ints_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 2) # no int attribute for p3
            self.assertEqual(n[0], 1)
            self.assertEqual(n[1], 2)
            n[0] = 42
            n[1] = 24
            self.assertEqual(p1.get_value(k), 42)
            self.assertEqual(p2.get_value(k), 24)

            n = m2._get_ints_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no int key for this model
        else:
            self.assertRaises(NotImplementedError, m1._get_ints_numpy, k)

    def test_get_spheres_numpy(self):
        """Test _get_spheres_numpy method"""
        m1 = IMP.Model("numpy get_spheres")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_spheres")
        p12 = IMP.Particle(m2)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        d1.set_radius(4)

        d2 = IMP.core.XYZR.setup_particle(p2)
        d2.set_coordinates(IMP.algebra.Vector3D(5,6,7))
        d2.set_radius(8)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            x, y, z, r = m1._get_spheres_numpy()
            for n in x, y, z, r:
                self.assertIs(n.base, m1)
                self.assertEqual(len(n), 2) # no sphere attribute for p3
            self.assertAlmostEqual(x[0], 1.0, delta=1e-4)
            self.assertAlmostEqual(y[0], 2.0, delta=1e-4)
            self.assertAlmostEqual(z[0], 3.0, delta=1e-4)
            self.assertAlmostEqual(r[0], 4.0, delta=1e-4)

            self.assertAlmostEqual(x[1], 5.0, delta=1e-4)
            self.assertAlmostEqual(y[1], 6.0, delta=1e-4)
            self.assertAlmostEqual(z[1], 7.0, delta=1e-4)
            self.assertAlmostEqual(r[1], 8.0, delta=1e-4)
            r[0] = 42.0
            self.assertAlmostEqual(d1.get_radius(), 42.0, delta=1e-6)
            x[1] = 24.0
            self.assertAlmostEqual(d2.get_coordinates()[0], 24.0, delta=1e-6)

            x, y, z, r = m2._get_spheres_numpy()
            for n in x, y, z, r:
                self.assertIs(n.base, m2)
                self.assertEqual(len(n), 0) # no spheres for this model
        else:
            self.assertRaises(NotImplementedError, m1._get_spheres_numpy)

    def test_get_sphere_derivatives_numpy(self):
        """Test _get_sphere_derivatives_numpy method"""
        m1 = IMP.Model("numpy get_sphere_derivatives")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_sphere_derivatives")
        p12 = IMP.Particle(m2)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            x, y, z, r = m1._get_sphere_derivatives_numpy()
            for n in x, y, z, r:
                self.assertIs(n.base, m1)
                self.assertEqual(len(n), 2) # no sphere attribute for p3
            x[0] = 42.0
            y[1] = 24.0
            self.assertAlmostEqual(d1.get_derivatives()[0], 42.0, delta=1e-6)
            self.assertAlmostEqual(d2.get_derivatives()[1], 24.0, delta=1e-6)
            x, y, z, r = m2._get_sphere_derivatives_numpy()
            for n in x, y, z, r:
                self.assertIs(n.base, m2)
                self.assertEqual(len(n), 0) # no spheres for this model
        else:
            self.assertRaises(NotImplementedError,
                              m1._get_sphere_derivatives_numpy)


if __name__ == '__main__':
    IMP.test.main()
