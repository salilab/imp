from __future__ import print_function
import IMP.test
import IMP.algebra
import io
import math
import sys
import pickle
import operator


class Tests(IMP.test.TestCase):

    def test_magnitude(self):
        """Check Vector3D magnitude"""
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(v.get_squared_magnitude(), 14.0)
        self.assertAlmostEqual(v.get_magnitude(), math.sqrt(14.0), places=1)

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE_AND_INTERNAL,
                     "No check in fast or release mode")
    def test_uninit(self):
        """Check use of uninitialized Vector3D"""
        v = IMP.algebra.Vector3D()
        self.assertRaises(IMP.InternalException, v.__getitem__, 0)

    def test_from_floats(self):
        """Check Vector3D from floats"""
        _ = IMP.algebra.Vector3D([1.0, 2.0, 3.0])

    def test_compare(self):
        """Test that vectors can't be compared"""
        v = IMP.algebra.Vector3D(0, 0, 0)
        self.assertRaises(ValueError, lambda a, b: a == b, v, v)

    def test_io(self):
        """Check I/O of Vector3Ds"""
        class NotAFile(object):
            pass
        vs1 = [
            IMP.algebra.Vector3D(
                1.0,
                2.0,
                3.0),
            IMP.algebra.Vector3D(
                4.0,
                5.0,
                6.0)]

        # Test read/write for regular files and file-like objects
        sio = io.BytesIO()
        IMP.algebra.write_pts(vs1, sio)
        sio.seek(0)
        rpts = IMP.algebra.read_pts(sio)
        self.assertEqual(len(rpts), len(vs1))
        for i in range(0, len(rpts)):
            for j in range(0, 3):
                self.assertAlmostEqual(rpts[i][j], vs1[i][j], delta=.01)
        sio = io.BytesIO(b"garbage")
        self.assertRaises(ValueError, IMP.algebra.read_pts, sio)

    def test_component(self):
        """Check Vector3D components"""
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(v[0], 1.0)
        self.assertEqual(v[1], 2.0)
        self.assertEqual(v[2], 3.0)

        self.assertEqual(v[-3], 1.0)
        self.assertEqual(v[-2], 2.0)
        self.assertEqual(v[-1], 3.0)
        v[0] = 10.0
        v[-1] = 30.0
        self.assertEqual(v[0], 10.0)
        self.assertEqual(v[2], 30.0)
        self.assertRaises(IndexError, lambda: v[3])
        self.assertRaises(IndexError, lambda: v[-4])

        def test_set(ind):
            v[ind] = 0.
        self.assertRaises(IndexError, test_set, 3)
        self.assertRaises(IndexError, test_set, -4)

    def test_to_list(self):
        """Check conversion of Vector3D to list"""
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        ls = list(v)
        self.assertEqual(len(ls), 3)
        self.assertAlmostEqual(ls[0], 1.0, delta=1e-6)
        self.assertAlmostEqual(ls[1], 2.0, delta=1e-6)
        self.assertAlmostEqual(ls[2], 3.0, delta=1e-6)

    def test_len(self):
        """Check Vector3D length"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(len(v1), 3)

    def test_dimension_mismatch(self):
        """Check failure to combine with other dimension vectors"""
        v3 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        v4 = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        k3 = IMP.algebra.VectorKD(v3)
        k4 = IMP.algebra.VectorKD(v4)
        # Should not be able to add 3D vector to 4D (or KD with K==4) vector
        self.assertRaises(TypeError, operator.add, v3, v4)
        self.assertRaises(TypeError, operator.add, v3, k4)
        self.assertRaises(TypeError, operator.add, v4, v3)
        self.assertRaises(TypeError, operator.add, v4, k3)
        # Should not be able to subtract 3D vector from
        # 4D (or KD with K==4) vector
        self.assertRaises(TypeError, operator.sub, v3, v4)
        self.assertRaises(TypeError, operator.sub, v3, k4)
        self.assertRaises(TypeError, operator.sub, v4, v3)
        self.assertRaises(TypeError, operator.sub, v4, k3)
        # Should not be able to get scalar product 3D vector with 4D
        self.assertRaises(TypeError, v3.get_scalar_product, v4)
        self.assertRaises(TypeError, v3.get_scalar_product, k4)
        self.assertRaises(TypeError, v4.get_scalar_product, v3)
        self.assertRaises(TypeError, v4.get_scalar_product, k3)
        # 3D vector with KD (with K==3) is OK, but scalar product is
        # not currently supported
        _ = v3 + k3
        _ = v3 - k3
        self.assertRaises(TypeError, v3.get_scalar_product, k3)
        self.assertRaises(TypeError, k3.get_scalar_product, v3)

    def test_scalar_product(self):
        """Check Vector3D scalar product"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.algebra.Vector3D(10.0, 1.0, 2.0)
        self.assertAlmostEqual(v1.get_scalar_product(v2), v2 * v1,
                               delta=.1)
        self.assertAlmostEqual(v1.get_scalar_product(v2), v1 * v2,
                               delta=.1)
        self.assertAlmostEqual(v1.get_scalar_product(v2), v2 * v1, delta=.1)
        self.assertAlmostEqual(v1.get_scalar_product(v2), 18.0, delta=.1)

    def test_product_scalar(self):
        """Check that multiplying vectors by scalars works"""
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        av = 3.0 * v
        va = 3.0 * v
        for i in range(0, 3):
            self.assertEqual(av[0], 3.0)
            self.assertEqual(va[0], 3.0)
            self.assertEqual(av[1], 6.0)
            self.assertEqual(va[1], 6.0)
            self.assertEqual(av[2], 9.0)
            self.assertEqual(va[2], 9.0)

    def test_vector_product(self):
        """Check Vector3D vector product"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.algebra.Vector3D(10.0, 1.0, 2.0)
        v12 = IMP.algebra.get_vector_product(v1, v2)
        v21 = IMP.algebra.get_vector_product(v2, v1)
        expected_v12 = (1.0, 28.0, -19.0)
        for i in range(3):
            self.assertAlmostEqual(v12[i], -v21[i], delta=.1)
            self.assertAlmostEqual(v12[i], expected_v12[i], delta=.1)

    def test_difference(self):
        """Check Vector3D difference"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.algebra.Vector3D(10.0, 1.0, 2.0)
        diff = v1 - v2
        v1 -= v2
        expected_diff = IMP.algebra.Vector3D(-9.0, 1.0, 1.0)
        self.assertAlmostEqual((diff - expected_diff).get_magnitude(),
                               0, delta=.1)
        self.assertAlmostEqual((v1 - expected_diff).get_magnitude(),
                               0, delta=.1)

    def test_get_distance(self):
        """Check Vector3D.get_distance()"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.algebra.Vector3D(10.0, 1.0, 2.0)
        dist = v1.get_distance(v2)
        self.assertAlmostEqual(dist, 9.11, delta=0.01)
        dist2 = (v1 - v2).get_magnitude()
        self.assertAlmostEqual(dist, dist2, delta=0.01)
        # Should not be able to get distance between different
        # dimension vectors
        v4 = IMP.algebra.Vector4D(10.0, 1.0, 2.0, 0.0)
        self.assertRaises(TypeError, v1.get_distance, v4)

    def test_show(self):
        """Check vector 3D show"""
        v = IMP.algebra.Vector3D(1, 2, 3)
        if sys.version_info[0] >= 3:
            out = io.StringIO()
        else:
            out = io.BytesIO()
        print(v, file=out)
        self.assertEqual(out.getvalue().find("Swig"), -1)

    def test_addition(self):
        """Check Vector3D addition"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        idv1 = id(v1)
        cppobj = str(v1.this)
        v2 = IMP.algebra.Vector3D(10.0, 1.0, 2.0)
        sum = v1 + v2
        v1 += v2
        # Inplace addition should not change the Python object identity:
        self.assertEqual(id(v1), idv1)
        # The underlying C++ object pointer should be unchanged too:
        self.assertEqual(str(v1.this), cppobj)
        expected_sum = IMP.algebra.Vector3D(11.0, 3.0, 5.0)
        self.assertAlmostEqual((sum - expected_sum).get_magnitude(),
                               0, delta=.1)
        self.assertAlmostEqual((v1 - expected_sum).get_magnitude(),
                               0, delta=.1)

    def test_scalar_multiplication(self):
        """Check Vector3D multiplication by a scalar"""
        v1 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        idv1 = id(v1)
        s1 = 3.0
        prod = v1 * s1
        v1 *= s1
        # Inplace multiplication should not change the Python object identity:
        self.assertEqual(id(v1), idv1)
        expected_prod = (3.0, 6.0, 9.0)
        for i in range(3):
            self.assertAlmostEqual(prod[i], expected_prod[i],
                                   delta=.1)
            self.assertAlmostEqual(v1[i], expected_prod[i],
                                   delta=.1)

    def test_scalar_division(self):
        """Check Vector3D division by a scalar"""
        v1 = IMP.algebra.Vector3D(3.0, 6.0, 9.0)
        idv1 = id(v1)
        s1 = 3.0
        prod = v1 / s1
        v1 /= s1
        # Inplace division should not change the Python object identity:
        self.assertEqual(id(v1), idv1)
        expected_prod = (1.0, 2.0, 3.0)
        for i in range(3):
            self.assertEqual(prod[i], expected_prod[i])
            self.assertEqual(v1[i], expected_prod[i])

    def test_distance(self):
        """Check distance between two vectors"""
        v1 = IMP.algebra.Vector3D(3.0, 6.0, 9.0)
        v2 = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(IMP.algebra.get_squared_distance(v1, v2), 56)
        self.assertAlmostEqual(IMP.algebra.get_distance(v1, v2), 7.4833,
                               delta=0.01)

    def test_generators(self):
        """Check the Vector3D generators"""
        # test calling since it is a bit non-trivial in SWIG
        _ = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d())
        _ = IMP.algebra.get_random_vector_in(
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), 1))

    def test_orth_vector(self):
        """Check get_orthogonal_vector()"""
        v1 = IMP.algebra.Vector3D(3.0, 6.0, 9.0)
        v2 = IMP.algebra.get_orthogonal_vector(v1)
        self.assertLess(IMP.algebra.get_distance(
            v2, IMP.algebra.Vector3D(1, 1, -1)), 1e-4)
        v1 = IMP.algebra.Vector3D(0, 0, 0)
        v2 = IMP.algebra.get_orthogonal_vector(v1)
        self.assertLess(IMP.algebra.get_distance(v2, v1), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of Vector3D"""
        v1 = IMP.algebra.Vector3D(3.0, 6.0, 9.0)
        v2 = IMP.algebra.Vector3D(1., 2., 3.)
        v2.foo = 'bar'
        vdump = pickle.dumps((v1, v2))

        newv1, newv2 = pickle.loads(vdump)
        self.assertLess(IMP.algebra.get_distance(v1, newv1), 1e-4)
        self.assertLess(IMP.algebra.get_distance(v2, newv2), 1e-4)
        self.assertEqual(newv2.foo, 'bar')

        self.assertRaises(TypeError, v1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
