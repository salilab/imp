import unittest
import IMP
import IMP.test
import IMP.algebra
from math import pi

class rotation_operation_tests(IMP.test.TestCase):
    def test_rotation_operation(self):
        """test that rotation operations are present in algebra """
        m = IMP.algebra.Matrix2D(2,2)
        m.init_zeros()
        result = IMP.algebra.Matrix2D(2,2)
        IMP.algebra.rotate_matrix_2D(m,pi/4,result)
        mat = IMP.algebra.EulerMatrixZYZ()
        angles = IMP.algebra.EulerAnglesZYZ(pi/2,-pi/2,0) # -y
        r = IMP.algebra.RotationMatrix2D(pi)

if __name__ == '__main__':
    unittest.main()
