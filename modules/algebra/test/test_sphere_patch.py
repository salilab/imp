import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra
import IMP.display
import math
class Sphere3DPatchTests(IMP.test.TestCase):
    """Test rigid transformations"""

    def setUp(self):
        """Build a set of test vectors"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)

    def test_sphere_patch_construction(self):
        """Check that sphere are constructed correctly"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        radius=5.0
        sph = IMP.algebra.Sphere3D(center,radius)
        xy_plane = IMP.algebra.Plane3D(IMP.algebra.Vector3D(0.,0.,0.),IMP.algebra.Vector3D(0.,0.,1.))
        patch = IMP.algebra.Sphere3DPatch(sph,xy_plane)
        self.assertEqual(patch.get_plane().is_above(IMP.algebra.Vector3D(1.,0.,1.)),True)
        self.assertEqual(patch.get_contains(IMP.algebra.Vector3D(0.0,1.0,0.4)),True)
        self.assertEqual(patch.get_contains(IMP.algebra.Vector3D(3.0,3.0,8.0)),False)
        self.assertEqual(patch.get_contains(IMP.algebra.Vector3D(0.0,1.0,-0.4)),False)
if __name__ == '__main__':
    unittest.main()
