import unittest
import IMP.test
import IMP.algebra as alg
import math



def print_matrix2D(m):
    r1=range(m.get_start(0),m.get_finish(0)+1)
    r2=range(m.get_start(1),m.get_finish(1)+1)
    for i in r1:
        for j in r2:
            print m[i,j],
        print "\n"


class Matrix2DTests(IMP.test.TestCase):
    def test_sizes(self):
        """Check proper creation an resizing"""
        m = alg.matrix2D(3,2)
        self.assertEqual(m.get_columns(), 2)
        m.resize(5,7)
        self.assertEqual(m.get_rows(), 5)
        self.assertEqual(m.get_columns(), 7)

    def test_access(self):
        """Check matrix2D access"""
        m = alg.matrix2D(3,2)
        m[2,1]=34.5
        self.assertEqual(m[2,1],34.5)

    def test_matrices_operations(self):
        """Check matrix3D operations"""
        m1 = alg.matrix2D(2,2)
        m2 = alg.matrix2D(2,2)
        result = alg.matrix2D(2,2)
        m1[0,0]=1;m1[0,1]=2;m1[1,0]=3;m1[1,1]=4
        m2[0,0]=1;m2[0,1]=2;m2[1,0]=3;m2[1,1]=4
        result = m1 + m2
        self.assertEqual(result[0,0],2)
        self.assertEqual(result[0,1],4)
        self.assertEqual(result[1,0],6)
        self.assertEqual(result[1,1],8)
        result = m1 - m2
        self.assertEqual(result[0,0],0)
        self.assertEqual(result[0,1],0)
        self.assertEqual(result[1,0],0)
        self.assertEqual(result[1,1],0)
        result = m1 * m2
        self.assertEqual(result[0,0],1)
        self.assertEqual(result[0,1],4)
        self.assertEqual(result[1,0],9)
        self.assertEqual(result[1,1],16)
        result = m1 / m2
        self.assertEqual(result[0,0],1)
        self.assertEqual(result[0,1],1)
        self.assertEqual(result[1,0],1)
        self.assertEqual(result[1,1],1)

#     print_matrix2D(m1)
#     print_matrix2D(m2)
#     print_matrix2D(m1+m2)
#     print_matrix2D(m1-m2)
#     print_matrix2D(m1*m2)
#     print_matrix2D(m1/m2)


    def test_matrix_scalar_operations(self):
        """Check matrix2D and scalar operations"""
        m1 = alg.matrix2D(2,2)
        m1[0,0]=1;m1[0,1]=2;m1[1,0]=3;m1[1,1]=4
        result = alg.matrix2D(2,2)
        result = m1 * 3.0

        result = m1 + 3
        self.assertEqual(result[0,0],4)
        self.assertEqual(result[0,1],5)
        self.assertEqual(result[1,0],6)
        self.assertEqual(result[1,1],7)
        result = m1 - 3
        self.assertEqual(result[0,0],-2)
        self.assertEqual(result[0,1],-1)
        self.assertEqual(result[1,0],0)
        self.assertEqual(result[1,1],1)
        result = m1 * 3
        self.assertEqual(result[0,0],3)
        self.assertEqual(result[0,1],6)
        self.assertEqual(result[1,0],9)
        self.assertEqual(result[1,1],12)
        result = m1 / 3
        self.assertInTolerance(result[0,0],0.333,0.001)
        self.assertInTolerance(result[0,1],0.666,0.001)
        self.assertInTolerance(result[1,0],1.000,0.001)
        self.assertInTolerance(result[1,1],1.333,0.001)

#     print_matrix2D(m1-3)
#     print_matrix2D(m1+3)
#     print_matrix2D(m1*3)
#     print_matrix2D(m1/3)


if __name__ == '__main__':
    unittest.main()
