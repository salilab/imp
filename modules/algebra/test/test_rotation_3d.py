from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import numpy as np
import random
import math
import pickle


class TransformFunct:

    def __init__(self, x_index, q_index, x_base, q_base):
        self.xi = x_index
        self.x = x_base
        self.qi = q_index
        self.q = q_base

    def __call__(self, v):
        self.q[self.qi] = v
        uv = self.q.get_unit_vector()
        r = IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        rx = r.get_rotated(self.x)
        return rx[self.xi]

    def get_analytic_deriv(self):
        uv = self.q.get_unit_vector()
        r = IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        return r.get_jacobian_of_rotated(self.x, True)[self.xi][self.qi]


class TransformFunct2:

    def __init__(self, p_index, q_index, p_base, q_base):
        self.pi = p_index
        self.p = p_base
        self.qi = q_index
        self.q = q_base

    def __call__(self, v):
        self.q[self.qi] = v
        uv = self.q.get_unit_vector()
        q = IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        qp = (q * self.p).get_quaternion()
        return qp[self.pi]

    def get_analytic_deriv(self):
        uv = self.q.get_unit_vector()
        q = IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        return IMP.algebra.get_jacobian_of_composed_wrt_first(
            q, self.p, True)[self.pi][self.qi]


class TransformFunct3:

    def __init__(self, p_index, q_index, p_base, q_base):
        self.pi = p_index
        self.p = p_base
        self.qi = q_index
        self.q = q_base

    def __call__(self, v):
        self.p[self.qi] = v
        uv = self.p.get_unit_vector()
        p = IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        qp = (self.q * p).get_quaternion()
        return qp[self.pi]

    def get_analytic_deriv(self):
        uv = self.p.get_unit_vector()
        p = IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        return IMP.algebra.get_jacobian_of_composed_wrt_second(
            self.q, p, True)[self.pi][self.qi]


class Tests(IMP.test.TestCase):

    """Test rotations"""

    def test_get_random(self):
        """Check get_random_rotation_3d()"""
        center = IMP.algebra.get_identity_rotation_3d()
        r = IMP.algebra.get_random_rotation_3d(center, 0.1)
        self.assertLess(IMP.algebra.get_distance(center, r), 0.1)

    def test_axis_rotation(self):
        """Check random rotations about axis"""
        for ii in range(1, 10):
            axis = IMP.algebra.get_random_vector_on(
                IMP.algebra.get_unit_sphere_3d())
            angle = math.pi / ii  # random.uniform(-math.pi,math.pi)

            r = IMP.algebra.get_rotation_about_axis(axis, angle)
            ri = r.get_inverse()
            v_start = IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d())
            vt = v_start
            for i in range(2 * ii):
                vt = r.get_rotated(vt)
                if i == 0:
                    vti = ri.get_rotated(vt)
            self.assertAlmostEqual(IMP.algebra.get_distance(v_start, vt), 0.,
                                   delta=.1)
            self.assertAlmostEqual(vti[0], v_start[0], delta=.1)
            self.assertAlmostEqual(vti[1], v_start[1], delta=.1)
            self.assertAlmostEqual(vti[2], v_start[2], delta=.1)

    def test_rotation_matrix_conversion(self):
        """Check converting Rotation3D to rotation matrix"""
        r1 = IMP.algebra.Rotation3D(0.202607, 0.0324723, 0.0521312, -0.977332)
        r2 = IMP.algebra.Rotation3D(0.900969, 0.0318942, 0.0297037, -0.431689)
        for r in [r1, r2]:
            rot_mat = []
            for i in range(3):
                t = r.get_rotation_matrix_row(i)
                rot_mat += [t[0], t[1], t[2]]
            rot_copy = IMP.algebra.get_rotation_from_matrix(
                rot_mat[0], rot_mat[1], rot_mat[2],
                rot_mat[3], rot_mat[
                    4], rot_mat[5],
                rot_mat[6], rot_mat[7], rot_mat[8])
            comp = rot_copy * r.get_inverse()
            q = comp.get_quaternion()
            print("===q:", q)
            for j, e in enumerate([1, 0, 0, 0]):
                self.assertAlmostEqual(q[j], e, delta=.1)

    def test_rotation(self):
        """Check that the rotation inverse is an inverse"""
        axis = IMP.algebra.get_random_vector_on(
            IMP.algebra.get_unit_sphere_3d())
        m = random.uniform(-1, 1)
        mag = m * m + axis * axis
        mag = mag ** (.5)
        axis /= mag
        m /= mag
        r = IMP.algebra.Rotation3D(m, axis[0], axis[1], axis[2])
        ri = r.get_inverse()
        v = IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_bounding_box_3d())
        vt = r.get_rotated(v)
        vti = ri.get_rotated(vt)
        v.show()
        vti.show()
        vt.show()
        self.assertAlmostEqual(vti[0], v[0], delta=.1)
        self.assertAlmostEqual(vti[1], v[1], delta=.1)
        self.assertAlmostEqual(vti[2], v[2], delta=.1)

    def test_deriv(self):
        """Check the quaternion derivatives"""
        r = IMP.algebra.get_random_rotation_3d()
        x = IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_bounding_box_3d())
        for qi in range(0, 4):
            for xi in range(0, 3):
                print("qi=" + str(qi) + " and xi= " + str(xi))
                tf = TransformFunct(xi, qi, x, r.get_quaternion())
                d = IMP.test.numerical_derivative(
                    tf, r.get_quaternion()[qi], .01)
                print(d)
                ad = tf.get_analytic_deriv()
                print(ad)
                self.assertAlmostEqual(d, ad, delta=.05)

    def test_deriv_quaternion_product_first(self):
        """Check the derivative of composed quaternion wrt first quaternion."""
        q = IMP.algebra.get_random_rotation_3d()
        p = IMP.algebra.get_random_rotation_3d()
        for qi in range(0, 4):
            for pi in range(0, 4):
                print("qi=" + str(qi) + " and pi= " + str(pi))
                tf = TransformFunct2(pi, qi, p, q.get_quaternion())
                d = IMP.test.numerical_derivative(
                    tf, q.get_quaternion()[qi], .001)
                print(d)
                ad = tf.get_analytic_deriv()
                print(ad)
                self.assertAlmostEqual(d, ad, delta=.05)

    def test_deriv_quaternion_product_second(self):
        """Check the derivative of composed quaternion wrt second quaternion."""
        q = IMP.algebra.get_random_rotation_3d()
        p = IMP.algebra.get_random_rotation_3d()
        for qi in range(0, 4):
            for pi in range(0, 4):
                print("qi=" + str(qi) + " and pi= " + str(pi))
                tf = TransformFunct3(pi, qi, p.get_quaternion(), q)
                d = IMP.test.numerical_derivative(
                    tf, p.get_quaternion()[qi], .001)
                print(d)
                ad = tf.get_analytic_deriv()
                print(ad)
                self.assertAlmostEqual(d, ad, delta=.05)

    def test_get_rotated_adjoint(self):
        rot = IMP.algebra.get_random_rotation_3d()
        x = np.random.normal(size=3)
        Dy = np.random.normal(size=3)
        J = rot.get_jacobian_of_rotated(x)
        Dq = np.dot(np.array(J).T, Dy)
        Dx = rot.get_inverse() * Dy
        adjoints = rot.get_rotated_adjoint(x, Dy)
        self.assertSequenceAlmostEqual(list(adjoints[0]), list(Dx))
        self.assertSequenceAlmostEqual(list(adjoints[1]), Dq.tolist())

    def test_compose_adjoint(self):
        p = IMP.algebra.get_random_rotation_3d()
        q = IMP.algebra.get_random_rotation_3d()
        Dr = np.random.normal(size=4)
        Jp = IMP.algebra.get_jacobian_of_composed_wrt_first(p, q)
        Jq = IMP.algebra.get_jacobian_of_composed_wrt_second(p, q)
        Dp = np.dot(np.array(Jp).T, Dr)
        Dq = np.dot(np.array(Jq).T, Dr)
        adjoints = IMP.algebra.compose_adjoint(p, q, Dr)
        self.assertSequenceAlmostEqual(list(adjoints[0]), Dp.tolist())
        self.assertSequenceAlmostEqual(list(adjoints[1]), Dq.tolist())

    def test_rotation_between_vectors(self):
        """Check that the rotation between two vectors is correct"""
        axis = []
        axis.append(IMP.algebra.Vector3D(1.0, 0.0, 0.0))
        axis.append(IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        # to check the parallel case
        axis.append(IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        axis.append(IMP.algebra.Vector3D(0.0, 0.0, 1.0))
        # to check the antiparallel case
        axis.append(IMP.algebra.Vector3D(0.0, 0.0, -1.0))
        axis.append(IMP.algebra.get_random_vector_on(
            IMP.algebra.get_unit_sphere_3d()))
        axis.append(IMP.algebra.get_random_vector_on(
            IMP.algebra.get_unit_sphere_3d()))

        # Check the very nearly parallel case
        for xpow in range(1, 7):
            x = math.pow(10, -xpow)
            for ypow in range(1, 7):
                y = math.pow(10, -ypow)
                axis.append(IMP.algebra.Vector3D(0.0, 0.0, -1.0))
                axis.append(IMP.algebra.Vector3D(x, y, -1.0))
        for i in range(len(axis) - 1):
            rot = IMP.algebra.get_rotation_taking_first_to_second(
                axis[i], axis[i + 1])
            self.assertAlmostEqual(IMP.algebra.get_distance(rot * axis[i],
                                                            axis[i + 1]),
                                   0.0, delta=0.01)

    def test_get_axis_and_angle(self):
        """Check that the function get_axis_and_angle is correct"""
        # Test for non identity rotation:
        r0 = IMP.algebra.get_random_rotation_3d()
        aa = IMP.algebra.get_axis_and_angle(r0)
        axis = aa[0]
        angle = aa[1]
        r1 = IMP.algebra.get_rotation_about_axis(axis, angle)
        dQ = r0.get_quaternion() - r1.get_quaternion()
#        print "dQ = ", dQ, dQ.get_squared_magnitude()
        self.assertAlmostEqual(
            dQ.get_squared_magnitude(), 0.0,
            delta=.0001)

        # Test that [1,0,0],0.0 is returned for any identity rotation:
        r0 = IMP.algebra.get_rotation_about_axis(axis, 0.0)
        aa = IMP.algebra.get_axis_and_angle(r0)
        axis = aa[0]
        angle = aa[1]
        self.assertAlmostEqual(0.0, angle, delta=.0001)
        self.assertAlmostEqual((axis - [1, 0, 0]).get_magnitude(),
                               0.0, delta=.0001)

    def test_is_equal_between_rotations(self):
        """Check that two rotations are equal"""
        t1 = IMP.algebra.get_random_rotation_3d()
        t2 = t1
        t3 = t1.get_inverse()
        self.assertAlmostEqual(IMP.algebra.get_distance(t1, t2), 0, delta=.05)
        # hard to say anything about a rotation and its inverse
        # self.assert_(IMP.algebra.get_distance(t1,t3), .5, .05)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(IMP.algebra.Rotation3D(1, 0, 0, 0),
                                     IMP.algebra.Rotation3D(0, 0, 0, 1)), 1, delta=.05)

    def test_interpolate(self):
        """Check that rotations can be interpolated"""
        r0 = IMP.algebra.get_random_rotation_3d()
        r1 = IMP.algebra.get_random_rotation_3d()
        print("Inputs")
        r0.show()
        r1.show()
        r5 = IMP.algebra.get_interpolated(r0, r1, .5)
        d = r5 / r0
        r1p = d * d * r0
        print("final")
        r1.show()
        print("out")
        r1p.show()
        self.assertAlmostEqual(
            (r1.get_quaternion() - r1p.get_quaternion()
             ).get_squared_magnitude(),
            0,
            delta=.1)

    def test_about_point(self):
        """Test get_rotation_about_point()"""
        r0 = IMP.algebra.get_random_rotation_3d()
        v = IMP.algebra.Vector3D(1, 2, 3)
        t = IMP.algebra.get_rotation_about_point(v, r0)
        # Rotating a point about itself should not move it
        self.assertLess(IMP.algebra.get_distance(v, t * v), 1e-6)

    def test_pickle(self):
        """Check that rotations can be (un-)pickled"""
        t1 = IMP.algebra.get_random_rotation_3d()
        t2 = IMP.algebra.get_random_rotation_3d()
        t2.foo = 'bar'
        tdump = pickle.dumps((t1, t2))

        newt1, newt2 = pickle.loads(tdump)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(t1, newt1), 0, delta=.05)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(t2, newt2), 0, delta=.05)
        self.assertEqual(newt2.foo, 'bar')

        self.assertRaises(TypeError, t1._set_from_binary, 42)

    def test_fixed_xyz_pickle(self):
        """Check that FixedXYZ objects can be (un-)pickled"""
        f1 = IMP.algebra.FixedXYZ(1., 2., 3.)
        f2 = IMP.algebra.FixedXYZ(4., 5., 6.)
        f2.foo = 'bar'
        dump = pickle.dumps((f1, f2))

        newf1, newf2 = pickle.loads(dump)
        self.assertAlmostEqual(f1.get_x(), newf1.get_x(), delta=1e-4)
        self.assertAlmostEqual(f1.get_y(), newf1.get_y(), delta=1e-4)
        self.assertAlmostEqual(f1.get_z(), newf1.get_z(), delta=1e-4)
        self.assertAlmostEqual(f2.get_x(), newf2.get_x(), delta=1e-4)
        self.assertAlmostEqual(f2.get_y(), newf2.get_y(), delta=1e-4)
        self.assertAlmostEqual(f2.get_z(), newf2.get_z(), delta=1e-4)
        self.assertEqual(newf2.foo, 'bar')

        self.assertRaises(TypeError, f1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
