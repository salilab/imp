import IMP
import IMP.test
import IMP.algebra
import IMP.em
from io import BytesIO

class Tests(IMP.test.TestCase):

    def test_rasterization(self):
        """Test creation DensityMap from grid"""
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
        grid_slow = IMP.algebra.get_rasterized([g], [1.0], 1.0, bb)
        grid_fast = IMP.algebra.get_rasterized_fast([g], [1.0], 1.0, bb)
        # Make sure that the returned grids are usable in Python
        self.assertLess((grid_fast.get_origin() - t).get_magnitude(), 1e-4)
        self.assertLess((grid_slow.get_origin() - t).get_magnitude(), 1e-4)
        d_slow=IMP.em.create_density_map(grid_slow)
        d_fast=IMP.em.create_density_map(grid_fast)
        # Check returned density maps
        d_origin = IMP.algebra.Vector3D(1.5, 2.5, 3.5)
        self.assertLess((d_fast.get_origin() - d_origin).get_magnitude(), 1e-4)
        self.assertLess((d_slow.get_origin() - d_origin).get_magnitude(), 1e-4)
        self.assertAlmostEqual(d_fast.get_spacing(), 1.0, delta=1e-4)
        self.assertAlmostEqual(d_slow.get_spacing(), 1.0, delta=1e-4)

if __name__ == '__main__':
    IMP.test.main()
