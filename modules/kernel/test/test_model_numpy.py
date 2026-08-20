import IMP
import IMP.core
import IMP.test


class Tests(IMP.test.TestCase):

    def test_get_derivatives_numpy(self):
        """Test get_derivatives_numpy method"""
        m1 = IMP.Model("numpy get_derivs")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_derivs")
        p12 = IMP.Particle(m2)

        k = IMP.FloatKey("myf")
        p1.add_attribute(k, 1.0)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1.get_derivatives_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 1) # no derivatives for p2
            self.assertAlmostEqual(n[0], 0.0, delta=1e-6)
            n[0] = 42.0
            self.assertAlmostEqual(p1.get_derivative(k), 42.0, delta=1e-6)

            # Read-only array should raise ValueError on assignment
            n = m1.get_derivatives_numpy(k, read_only=True)
            self.assertRaises(ValueError, n.__setitem__, 0, 42.0)

            n = m2.get_derivatives_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no float key for this model
        else:
            self.assertRaises(NotImplementedError, m1.get_derivatives_numpy, k)

    def test_get_floats_numpy(self):
        """Test get_floats_numpy method"""
        m1 = IMP.Model("numpy get_floats")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_floats")
        p12 = IMP.Particle(m2)

        k = IMP.FloatKey("myf")
        p1.add_attribute(k, 1.0)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1.get_floats_numpy(k)
            n2 = m1.get_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 1) # no float attribute for p2
            self.assertAlmostEqual(n[0], 1.0, delta=1e-6)
            n[0] = 42.0
            # n2 should be the same view as n
            self.assertAlmostEqual(n2[0], 42.0, delta=1e-6)
            self.assertAlmostEqual(p1.get_value(k), 42.0, delta=1e-6)

            # Read-only array should raise ValueError on assignment
            n = m1.get_floats_numpy(k, read_only=True)
            self.assertRaises(ValueError, n.__setitem__, 0, 42.0)

            n = m2.get_floats_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no float key for this model
        else:
            self.assertRaises(NotImplementedError, m1.get_floats_numpy, k)

    def test_get_ints_numpy(self):
        """Test get_ints_numpy method"""
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
            n = m1.get_ints_numpy(k)
            n2 = m1.get_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 2) # no int attribute for p3
            self.assertEqual(n[0], 1)
            self.assertEqual(n[1], 2)
            n[0] = 42
            n[1] = 24
            # n2 should be the same view as n
            self.assertEqual(n2[0], 42)
            self.assertEqual(n2[1], 24)
            self.assertEqual(p1.get_value(k), 42)
            self.assertEqual(p2.get_value(k), 24)

            # Read-only array should raise ValueError on assignment
            n = m1.get_ints_numpy(k, read_only=True)
            self.assertRaises(ValueError, n.__setitem__, 0, 42)

            n = m2.get_ints_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no int key for this model
        else:
            self.assertRaises(NotImplementedError, m1.get_ints_numpy, k)

    def test_get_spheres_numpy(self):
        """Test get_spheres_numpy method"""
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
            c, r = m1.get_spheres_numpy()
            for n in c, r:
                self.assertIs(n.base, m1)
                self.assertEqual(len(n), 2) # no sphere attribute for p3
            self.assertAlmostEqual(c[0][0], 1.0, delta=1e-4)
            self.assertAlmostEqual(c[0][1], 2.0, delta=1e-4)
            self.assertAlmostEqual(c[0][2], 3.0, delta=1e-4)
            self.assertAlmostEqual(r[0], 4.0, delta=1e-4)

            self.assertAlmostEqual(c[1][0], 5.0, delta=1e-4)
            self.assertAlmostEqual(c[1][1], 6.0, delta=1e-4)
            self.assertAlmostEqual(c[1][2], 7.0, delta=1e-4)
            self.assertAlmostEqual(r[1], 8.0, delta=1e-4)
            r[0] = 42.0
            self.assertAlmostEqual(d1.get_radius(), 42.0, delta=1e-6)
            c[1][0] = 24.0
            self.assertAlmostEqual(d2.get_coordinates()[0], 24.0, delta=1e-6)

            # Read-only array should raise ValueError on assignment
            c, r = m1.get_spheres_numpy(read_only=True)
            self.assertRaises(ValueError, r.__setitem__, 0, 42.0)
            self.assertRaises(ValueError, c[1].__setitem__, 0, 24.0)

            c, r = m2.get_spheres_numpy()
            for n in c, r:
                self.assertIs(n.base, m2)
                self.assertEqual(len(n), 0) # no spheres for this model
        else:
            self.assertRaises(NotImplementedError, m1.get_spheres_numpy)

    def test_get_sphere_derivatives_numpy(self):
        """Test get_sphere_derivatives_numpy method"""
        m1 = IMP.Model("numpy get_sphere_derivatives")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_sphere_derivatives")
        p12 = IMP.Particle(m2)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            c, r = m1.get_sphere_derivatives_numpy()
            for n in c, r:
                self.assertIs(n.base, m1)
                self.assertEqual(len(n), 2) # no sphere attribute for p3
            c[0][0] = 42.0
            c[1][1] = 24.0
            self.assertAlmostEqual(d1.get_derivatives()[0], 42.0, delta=1e-6)
            self.assertAlmostEqual(d2.get_derivatives()[1], 24.0, delta=1e-6)

            # Read-only array should raise ValueError on assignment
            c, r = m1.get_sphere_derivatives_numpy(read_only=True)
            self.assertRaises(ValueError, r.__setitem__, 0, 42.0)
            self.assertRaises(ValueError, c[1].__setitem__, 0, 24.0)

            c, r = m2.get_sphere_derivatives_numpy()
            for n in c, r:
                self.assertIs(n.base, m2)
                self.assertEqual(len(n), 0) # no spheres for this model
        else:
            self.assertRaises(NotImplementedError,
                              m1.get_sphere_derivatives_numpy)

    def test_get_internal_coordinates_numpy(self):
        """Test get_internal_coordinates_numpy method"""
        m1 = IMP.Model("numpy get_internal_coordinates")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_internal_coordinates")
        p12 = IMP.Particle(m2)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        d1.set_radius(4)

        rb2 = IMP.core.RigidBody.setup_particle(p2, [p1])

        if IMP.IMP_KERNEL_HAS_NUMPY:
            c = m1.get_internal_coordinates_numpy()
            self.assertIs(c.base, m1)
            self.assertEqual(len(c), 1) # no intcoord attribute for p2, p3
            # Internal coordinates should be zero since rb2 will be
            # colocated with p1
            self.assertAlmostEqual(c[0][0], 0.0, delta=1e-4)
            self.assertAlmostEqual(c[0][1], 0.0, delta=1e-4)
            self.assertAlmostEqual(c[0][2], 0.0, delta=1e-4)

            c[0][0] = 24.0
            # Force update of global coords from internal coords
            m1.update()
            self.assertAlmostEqual(d1.get_coordinates()[0], 25.0, delta=1e-4)

            # Read-only array should raise ValueError on assignment
            c = m1.get_internal_coordinates_numpy(read_only=True)
            self.assertRaises(ValueError, c[0].__setitem__, 0, 24.0)

            c = m2.get_internal_coordinates_numpy()
            self.assertIs(c.base, m2)
            self.assertEqual(len(c), 0) # no int coords for this model
        else:
            self.assertRaises(NotImplementedError,
                              m1.get_internal_coordinates_numpy)

    def test_get_internal_coordinate_derivatives_numpy(self):
        """Test get_internal_coordinate_derivatives_numpy method"""
        m1 = IMP.Model("numpy get_internal_coordinate_derivatives")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_internal_coordinate_derivatives")
        p12 = IMP.Particle(m2)

        d1 = IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        d1.set_radius(4)

        rb2 = IMP.core.RigidBody.setup_particle(p2, [p1])

        if IMP.IMP_KERNEL_HAS_NUMPY:
            c = m1.get_internal_coordinate_derivatives_numpy()
            self.assertIs(c.base, m1)
            self.assertEqual(len(c), 1) # no intcoord attribute for p2, p3
            c[0][0] = 24.0

            # Read-only array should raise ValueError on assignment
            c = m1.get_internal_coordinate_derivatives_numpy(read_only=True)
            self.assertRaises(ValueError, c[0].__setitem__, 0, 24.0)

            c = m2.get_internal_coordinate_derivatives_numpy()
            self.assertIs(c.base, m2)
            self.assertEqual(len(c), 0) # no int coords for this model
        else:
            self.assertRaises(NotImplementedError,
                              m1.get_internal_coordinate_derivatives_numpy)

    def test_get_numpy_vector3d(self):
        """Test get_numpy method for Vector3DKey"""
        k = IMP.Vector3DKey("test v3dkey")
        m1, m2, p1, p2, p3 = self._get_numpy_vectornd(IMP.Vector3D, 3, k)

    def test_numpy_vector3d_deriv(self):
        """Test numpy methods for Vector3DDerivKey"""
        k = IMP.Vector3DDerivKey("test v3dderivkey")
        self._test_numpy_vector_nd_deriv(IMP.Vector3D, 3, k)

    def test_get_numpy_vector4d(self):
        """Test get_numpy method for Vector4DKey"""
        k = IMP.Vector4DKey("test v4dkey")
        m1, m2, p1, p2, p3 = self._get_numpy_vectornd(IMP.Vector4D, 4, k)

    def test_numpy_vector4d_deriv(self):
        """Test numpy methods for Vector4DDerivKey"""
        k = IMP.Vector4DDerivKey("test v4dderivkey")
        self._test_numpy_vector_nd_deriv(IMP.Vector4D, 4, k)

    def _test_numpy_vector_nd_deriv(self, vector_cls, n, k):
        m1, m2, p1, p2, p3 = self._get_numpy_vectornd(vector_cls, n, k)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            c = m1.get_derivatives_numpy(k)
            self.assertIs(c.base, m1)
            self.assertEqual(len(c), 2) # no VectorD derivatives for p3
            self.assertAlmostEqual(c[0][0], 0.0, delta=1e-4)
            self.assertAlmostEqual(c[0][1], 0.0, delta=1e-4)
            self.assertAlmostEqual(c[0][2], 0.0, delta=1e-4)

            # Read-only array should raise ValueError on assignment
            n = m1.get_derivatives_numpy(k, read_only=True)
            self.assertRaises(ValueError, n.__setitem__, 0, 42.0)

    def _get_numpy_vectornd(self, vector_cls, n, k):
        m1 = IMP.Model("numpy VectorDKey")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no VectorDKey")
        p12 = IMP.Particle(m2)

        m1.add_attribute(k, p1, vector_cls(*range(1, 1 + n)))
        m1.add_attribute(k, p2, vector_cls(*range(5, 5 + n)))

        if IMP.IMP_KERNEL_HAS_NUMPY:
            c = m1.get_numpy(k)
            self.assertIs(c.base, m1)
            self.assertEqual(c.shape, (2, n)) # no VectorD attribute for p3
            self.assertAlmostEqual(c[0][0], 1.0, delta=1e-4)
            self.assertAlmostEqual(c[0][1], 2.0, delta=1e-4)
            self.assertAlmostEqual(c[0][2], 3.0, delta=1e-4)

            self.assertAlmostEqual(c[1][0], 5.0, delta=1e-4)
            self.assertAlmostEqual(c[1][1], 6.0, delta=1e-4)
            self.assertAlmostEqual(c[1][2], 7.0, delta=1e-4)
            c[1][0] = 24.0
            self.assertAlmostEqual(m1.get_attribute(k, p2)[0],
                                   24.0, delta=1e-6)

            # Read-only array should raise ValueError on assignment
            c = m1.get_numpy(k, read_only=True)
            self.assertRaises(ValueError, c[1].__setitem__, 0, 24.0)

            c = m2.get_numpy(k)
            self.assertIs(c.base, m2)
            self.assertEqual(len(c), 0) # no Vector3D for this model
        return m1, m2, p1, p2, p3

    def test_numpy_particle_indexes(self):
        """Test numpy methods for ParticleIndexKey"""
        m1 = IMP.Model("numpy particle_index")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no particle_index")
        p12 = IMP.Particle(m2)

        k = IMP.ParticleIndexKey("myf")
        p1.add_attribute(k, p1)
        p2.add_attribute(k, p2)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1.get_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 2) # no ParticleIndex attribute for p3
            self.assertEqual(n[0], p1.get_index())
            self.assertEqual(n[1], p2.get_index())
            n[0] = p2.get_index()
            n[1] = p1.get_index()
            self.assertEqual(p1.get_value(k), p2)
            self.assertEqual(p2.get_value(k), p1)

            # Read-only array should raise ValueError on assignment
            n = m1.get_numpy(k, read_only=True)
            self.assertRaises(ValueError, n.__setitem__, 0, p1.get_index())

            n = m2.get_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no ParticleIndex key for this model


if __name__ == '__main__':
    IMP.test.main()
