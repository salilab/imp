import IMP
import IMP.test
import IMP.algebra
from io import BytesIO
import pickle


class Tests(IMP.test.TestCase):

    def test_constructor(self):
        """Test creation of Gaussian3D"""
        t = IMP.algebra.Vector3D(1., 2., 3.)
        r = IMP.algebra.get_identity_rotation_3d()
        tran = IMP.algebra.Transformation3D(r, t)
        rf = IMP.algebra.ReferenceFrame3D(tran)
        var = IMP.algebra.Vector3D(4., 5., 6.)
        g = IMP.algebra.Gaussian3D(rf, var)
        sio = BytesIO()
        g.show(sio)

        def check_gauss(g):
            g_rf = g.get_reference_frame()
            self.assertLess(IMP.algebra.get_distance(
                g_rf.get_transformation_to().get_translation(), t), 1e-4)
            self.assertLess(IMP.algebra.get_distance(g.get_variances(), var),
                            1e-4)
            self.assertLess(IMP.algebra.get_distance(g.get_center(), t), 1e-4)
        check_gauss(g)

        covar = IMP.algebra.get_covariance(g)
        g2 = IMP.algebra.get_gaussian_from_covariance(covar, t)
        check_gauss(g2)
        bb = IMP.algebra.BoundingBox3D(t, t)
        grid = IMP.algebra.get_rasterized([g], [1.0], 1.0, bb)
        grid_fast = IMP.algebra.get_rasterized_fast([g], [1.0], 1.0, bb)

    def test_pickle(self):
        """Test (un-)pickle of Gaussian3D"""
        t1 = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
        t2 = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
        g1 = IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(t1),
                                    IMP.algebra.Vector3D(1., 2., 3.))
        g2 = IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(t2),
                                    IMP.algebra.Vector3D(3., 4., 5.))

        g2.foo = 'bar'
        dump = pickle.dumps((g1, g2))

        newg1, newg2 = pickle.loads(dump)
        self._assert_equal(g1, newg1)
        self._assert_equal(g2, newg2)
        self.assertEqual(newg2.foo, 'bar')

    def _assert_equal(self, a, b):
        ta = a.get_reference_frame().get_transformation_to()
        tb = b.get_reference_frame().get_transformation_to()
        self.assertLess(
            IMP.algebra.get_distance(ta.get_rotation(),
                                     tb.get_rotation()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(ta.get_translation(),
                                     tb.get_translation()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            a.get_variances(), b.get_variances()), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
