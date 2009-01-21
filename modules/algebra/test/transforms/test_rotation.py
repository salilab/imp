import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra
import random

class RigidTransformationTests(IMP.test.TestCase):
    """Test particles"""

    def test_rotation(self):
        """Check that the rotation inverse is an inverse"""
        axis= IMP.algebra.random_vector_on_unit_sphere()
        m= random.uniform(-1,1)
        mag= m*m+ axis*axis
        mag = mag**(.5)
        axis/= mag
        m/=mag
        r= IMP.algebra.Rotation3D(m, axis[0], axis[1], axis[2])
        ri= r.get_inverse()
        v= IMP.algebra.random_vector_in_unit_box()
        vt= r.rotate(v)
        vti= ri.rotate(vt)
        v.show()
        vti.show()
        vt.show()
        self.assertInTolerance(vti[0], v[0], .1)
        self.assertInTolerance(vti[1], v[1], .1)
        self.assertInTolerance(vti[2], v[2], .1)

if __name__ == '__main__':
    unittest.main()
