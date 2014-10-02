import IMP
import IMP.test
import IMP.algebra
from StringIO import StringIO

class Tests(IMP.test.TestCase):

    def test_constructor(self):
        """Test creation of Gaussian3D"""
        t = IMP.algebra.Vector3D(1., 2., 3.)
        r = IMP.algebra.get_identity_rotation_3d()
        tran = IMP.algebra.Transformation3D(r, t)
        rf = IMP.algebra.ReferenceFrame3D(tran)
        var = IMP.algebra.Vector3D(4., 5., 6.)
        g = IMP.algebra.Gaussian3D(rf, var)
        sio = StringIO()
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

if __name__ == '__main__':
    IMP.test.main()
