import unittest
import IMP.test
import IMP.algebra as alg
import math



def print_matrix3D(m):

    for i in xrange(m.get_start(0),m.get_finish(0)+1):
        print "Layer :",i
        for j in xrange(m.get_start(1),m.get_finish(1)+1):
            for k in xrange(m.get_start(2),m.get_finish(2)+1):
                print m[i,j,k],
            print "\n"

class Matrix3DTests(IMP.test.TestCase):
    def test_sizes(self):
        """Check proper creation an resizing"""
        m = alg.matrix3D(3,2,3)
        self.assertEqual(m.get_slices(), 3)
        m.resize(5,7,6)
        self.assertEqual(m.get_slices(), 5)
        self.assertEqual(m.get_rows(), 7)
        self.assertEqual(m.get_columns(), 6)

    def test_access(self):
        """Check matrix3D access"""
        m = alg.matrix3D(3,2,7)
        m[2,1,6]=34.5
        self.assertEqual(m[2,1,6],34.5)

    def test_matrices_operations(self):
        """Check matrix3D operations"""
        m1 = alg.matrix3D(2,2,4)
        m2 = alg.matrix3D(2,2,4)
        result = alg.matrix3D(2,2,4)

        for i in xrange(m1.get_start(0),m1.get_finish(0)+1):
            for j in xrange(m1.get_start(1),m1.get_finish(1)+1):
                for k in xrange(m1.get_start(2),m1.get_finish(2)+1):
                    m1[i,j,k]=i+j+k
                    m2[i,j,k]=i+j+k-1

        result = m1 + m2
        self.assertEqual(result[0,0,0],-1)
        self.assertEqual(result[0,0,1],1)
        self.assertEqual(result[0,0,2],3)
        self.assertEqual(result[0,0,3],5)
        self.assertEqual(result[0,1,0],1)
        self.assertEqual(result[0,1,1],3)
        self.assertEqual(result[0,1,2],5)
        self.assertEqual(result[0,1,3],7)
        self.assertEqual(result[1,0,0],1)
        self.assertEqual(result[1,0,1],3)
        self.assertEqual(result[1,0,2],5)
        self.assertEqual(result[1,0,3],7)
        self.assertEqual(result[1,1,0],3)
        self.assertEqual(result[1,1,1],5)
        self.assertEqual(result[1,1,2],7)
        self.assertEqual(result[1,1,3],9)

        result = m1 - m2
        self.assertEqual(result[0,0,0],1)
        self.assertEqual(result[0,0,1],1)
        self.assertEqual(result[0,0,2],1)
        self.assertEqual(result[0,0,3],1)
        self.assertEqual(result[0,1,0],1)
        self.assertEqual(result[0,1,1],1)
        self.assertEqual(result[0,1,2],1)
        self.assertEqual(result[0,1,3],1)
        self.assertEqual(result[1,0,0],1)
        self.assertEqual(result[1,0,1],1)
        self.assertEqual(result[1,0,2],1)
        self.assertEqual(result[1,0,3],1)
        self.assertEqual(result[1,1,0],1)
        self.assertEqual(result[1,1,1],1)
        self.assertEqual(result[1,1,2],1)
        self.assertEqual(result[1,1,3],1)
        result = m1 * m2
        self.assertEqual(result[0,0,0],0)
        self.assertEqual(result[0,0,1],0)
        self.assertEqual(result[0,0,2],2)
        self.assertEqual(result[0,0,3],6)
        self.assertEqual(result[0,1,0],0)
        self.assertEqual(result[0,1,1],2)
        self.assertEqual(result[0,1,2],6)
        self.assertEqual(result[0,1,3],12)
        self.assertEqual(result[1,0,0],0)
        self.assertEqual(result[1,0,1],2)
        self.assertEqual(result[1,0,2],6)
        self.assertEqual(result[1,0,3],12)
        self.assertEqual(result[1,1,0],2)
        self.assertEqual(result[1,1,1],6)
        self.assertEqual(result[1,1,2],12)
        self.assertEqual(result[1,1,3],20)
        result = m1 / m2
        self.assertInTolerance(result[0,0,0],0,0.001)
#    self.assertInTolerance(result[0,0,1],inf,0.001)
        self.assertInTolerance(result[0,0,2],2,0.001)
        self.assertInTolerance(result[0,0,3],1.5,0.001)
#    self.assertInTolerance(result[0,1,0],inf,.001)
        self.assertInTolerance(result[0,1,1],2.0,0.001)
        self.assertInTolerance(result[0,1,2],1.5,0.001)
        self.assertInTolerance(result[0,1,3],1.333,0.001)
#    self.assertInTolerance(result[1,0,0],inf,0.001)
        self.assertInTolerance(result[1,0,1],2,0.001)
        self.assertInTolerance(result[1,0,2],1.5,0.001)
        self.assertInTolerance(result[1,0,3],1.333,0.001)
        self.assertInTolerance(result[1,1,0],2,0.001)
        self.assertInTolerance(result[1,1,1],1.5,0.001)
        self.assertInTolerance(result[1,1,2],1.333,0.001)
        self.assertInTolerance(result[1,1,3],1.25,0.001)

#     print "m1"
#     print_matrix3D(m1)
#     print "m2"
#     print_matrix3D(m2)
#     print "m1+m2"
#     print_matrix3D(m1+m2)
#     print "m1-m2"
#     print_matrix3D(m1-m2)
#     print "m1*m2"
#     print_matrix3D(m1*m2)
#     print "m1/m2"
#     print_matrix3D(m1/m2)


    def test_matrix_scalar_operations(self):
        """Check matrix3D and scalar operations"""
        """Check matrix3D operations"""
        m1 = alg.matrix3D(2,2,4)
        result = alg.matrix3D(2,2,4)

        for i in xrange(m1.get_start(0),m1.get_finish(0)+1):
            for j in xrange(m1.get_start(1),m1.get_finish(1)+1):
                for k in xrange(m1.get_start(2),m1.get_finish(2)+1):
                    m1[i,j,k]=i+j+k

        result = m1 - 3
        self.assertEqual(result[0,0,0],-3)
        self.assertEqual(result[0,0,1],-2)
        self.assertEqual(result[0,0,2],-1)
        self.assertEqual(result[0,0,3],0)
        self.assertEqual(result[0,1,0],-2)
        self.assertEqual(result[0,1,1],-1)
        self.assertEqual(result[0,1,2],0)
        self.assertEqual(result[0,1,3],1)
        self.assertEqual(result[1,0,0],-2)
        self.assertEqual(result[1,0,1],-1)
        self.assertEqual(result[1,0,2],0)
        self.assertEqual(result[1,0,3],1)
        self.assertEqual(result[1,1,0],-1)
        self.assertEqual(result[1,1,1],0)
        self.assertEqual(result[1,1,2],1)
        self.assertEqual(result[1,1,3],2)

        result = m1 + 3
        self.assertEqual(result[0,0,0],3)
        self.assertEqual(result[0,0,1],4)
        self.assertEqual(result[0,0,2],5)
        self.assertEqual(result[0,0,3],6)
        self.assertEqual(result[0,1,0],4)
        self.assertEqual(result[0,1,1],5)
        self.assertEqual(result[0,1,2],6)
        self.assertEqual(result[0,1,3],7)
        self.assertEqual(result[1,0,0],4)
        self.assertEqual(result[1,0,1],5)
        self.assertEqual(result[1,0,2],6)
        self.assertEqual(result[1,0,3],7)
        self.assertEqual(result[1,1,0],5)
        self.assertEqual(result[1,1,1],6)
        self.assertEqual(result[1,1,2],7)
        self.assertEqual(result[1,1,3],8)

        result = m1 * 3
        self.assertEqual(result[0,0,0],0)
        self.assertEqual(result[0,0,1],3)
        self.assertEqual(result[0,0,2],6)
        self.assertEqual(result[0,0,3],9)
        self.assertEqual(result[0,1,0],3)
        self.assertEqual(result[0,1,1],6)
        self.assertEqual(result[0,1,2],9)
        self.assertEqual(result[0,1,3],12)
        self.assertEqual(result[1,0,0],3)
        self.assertEqual(result[1,0,1],6)
        self.assertEqual(result[1,0,2],9)
        self.assertEqual(result[1,0,3],12)
        self.assertEqual(result[1,1,0],6)
        self.assertEqual(result[1,1,1],9)
        self.assertEqual(result[1,1,2],12)
        self.assertEqual(result[1,1,3],15)

        result = m1 / 3
        self.assertInTolerance(result[0,0,0],0.000,0.001)
        self.assertInTolerance(result[0,0,1],0.333,0.001)
        self.assertInTolerance(result[0,0,2],0.666,0.001)
        self.assertInTolerance(result[0,0,3],1.000,0.001)
        self.assertInTolerance(result[0,1,0],0.333,0.001)
        self.assertInTolerance(result[0,1,1],0.666,0.001)
        self.assertInTolerance(result[0,1,2],1.000,0.001)
        self.assertInTolerance(result[0,1,3],1.333,0.001)
        self.assertInTolerance(result[1,0,0],0.333,0.001)
        self.assertInTolerance(result[1,0,1],0.666,0.001)
        self.assertInTolerance(result[1,0,2],1.000,0.001)
        self.assertInTolerance(result[1,0,3],1.333,0.001)
        self.assertInTolerance(result[1,1,0],0.666,0.001)
        self.assertInTolerance(result[1,1,1],1.000,0.001)
        self.assertInTolerance(result[1,1,2],1.333,0.001)
        self.assertInTolerance(result[1,1,3],1.666,0.001)

#     print_matrix3D(m1-3)
#     print_matrix3D(m1+3)
#     print_matrix3D(m1*3)
#     print_matrix3D(m1/3)



if __name__ == '__main__':
    unittest.main()
