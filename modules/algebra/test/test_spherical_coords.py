import IMP
import IMP.test
import IMP.algebra
from math import pi

class Tests(IMP.test.TestCase):
    def test_spherical_coords_conversions(self):
        """Check that spherical coords work"""
        r = IMP.algebra.SphericalVector3D(1.,pi/2,0.0)
        p = r.get_cartesian_coordinates()
        self.assertAlmostEqual(p[0],1.0,places=3)
        self.assertAlmostEqual(p[1],0.0,places=3)
        self.assertAlmostEqual(p[2],0.0,places=3)
        x = IMP.algebra.Vector3D(1.,0.,0.)
        r = IMP.algebra.SphericalVector3D(x)
        self.assertAlmostEqual(r[0],1.0,places=3)
        self.assertAlmostEqual(r[1],pi/2.,places=3)
        self.assertAlmostEqual(r[2],0.,places=3)

if __name__ == '__main__':
    IMP.test.main()
