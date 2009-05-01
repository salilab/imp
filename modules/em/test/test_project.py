import unittest
import random
import math
import IMP.test
import IMP.algebra
import IMP.em

def print_matrix3D(m):
    for i in range(m.get_start(0),m.get_finish(0)+1):
        print "Layer :",i
        for j in range(m.get_start(1),m.get_finish(1)+1):
            for k in range(m.get_start(2),m.get_finish(2)+1):
                print m[i,j,k],
            print "\n"

def print_matrix2D(m,name="None"):
    print " ==============================================="
    print name
    r1=range(m.get_start(0),m.get_finish(0)+1)
    r2=range(m.get_start(1),m.get_finish(1)+1)
    for i in r1:
        for j in r2:
            print m[i,j],
        print "\n"
    print "==============================================="


def create_matrix(z,y,x):
    m1 = IMP.algebra.Matrix3D(z,y,x)
    for i in range(m1.get_start(0),m1.get_finish(0)+1):
        for j in range(m1.get_start(1),m1.get_finish(1)+1):
            for k in range(m1.get_start(2),m1.get_finish(2)+1):
                m1[i,j,k]=i+j+k
#    print_matrix3D(m1)
#    print "==============================================="
    return m1

def create_cube(z,y,x):
    m1 = IMP.algebra.Matrix3D(z,y,x)
    m1.centered_start()
    for i in range(-2,2+1):
        for j in range(-2,2+1):
            for k in range(-2,2+1):
                m1[i,j,k]=random.random()
    m1.set_start(0,0)
    m1.set_start(1,0)
    m1.set_start(2,0)
#    print_matrix3D(m1)
#    print "==============================================="
    return m1


def make_matrix(v):
    m = IMP.algebra.Matrix2D(len(v), len(v[0]))
    for row in range(len(v)):
        for col in range(len(v[0])):
            m[row,col] = v[row][col]
    return m

class EMprojectTests(IMP.test.TestCase):

    def test_project_Z(self):
        """Check projection of a matrix3D for axis Z"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        angles = IMP.algebra.EulerAnglesZYZ(0,0,0) # Z
        IMP.em.project_given_euler_angles1(m1,result,dy,dx,
                                          angles,shift,tolerance)
#        print_matrix2D(result,"test_project_Z")
        for i in range(0,dy):
            for j in range(0,dx):
                self.assertEqual(result[i,j],3*(i+j+1))

    def test_project_Z_opposite(self):
        """Check projection of a matrix3D for axis -Z"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        angles = IMP.algebra.EulerAnglesZYZ(0,math.pi,0) # -z
        IMP.em.project_given_euler_angles1(m1,
                          result,dy,dx,angles,shift ,tolerance)
#        print_matrix2D(result,"test_project_-Z")
        for i in range(0,dy):
            for j in range(0,dx):
                self.assertEqual(result[i,(dx-1)-j],3*(i+j+1))


    def test_project_X(self):
        """Check projection of a matrix3D for axis X"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        angles = IMP.algebra.EulerAnglesZYZ(0,-math.pi/2,0) # x
        IMP.em.project_given_euler_angles1(m1,result,dy,dz,
                                          angles,shift ,tolerance)
#        print_matrix2D(result,"test_project_X")
        for i in range(0,dy):
            for j in range(0,dz):
                self.assertEqual(result[i,j],9*(4+i+dz-1-j))


    def test_project_X_opposite(self):
        """Check projection of a matrix3D for axis -X"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        angles = IMP.algebra.EulerAnglesZYZ(0,math.pi/2,0) # -x
        IMP.em.project_given_euler_angles1(m1,result,dy,dz,
                                          angles,shift ,tolerance)
#        print_matrix2D(result,"test_project_-X")
        for i in range(0,dy):
            for j in range(0,dz):
                self.assertEqual(result[i,dz-1-j],9*(4+i+dz-1-j))


    def test_project_Y(self):
        """Check projection of a matrix3D for axis Y"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        angles = IMP.algebra.EulerAnglesZYZ(math.pi/2,math.pi/2,0) # y
        IMP.em.project_given_euler_angles1(m1,result,dx,dz,
                                          angles,shift ,tolerance)
#        print_matrix2D(result,"test_project_Y")
        for i in range(0,dx):
            for j in range(0,dz):
                self.assertEqual(result[i,j],5*(i+j+2))


    def test_project_Y_opposite(self):
        """Check projection of a matrix3D for axis -Y"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        angles = IMP.algebra.EulerAnglesZYZ(math.pi/2,-math.pi/2,0) # -y
        IMP.em.project_given_euler_angles1(m1,result,dx,dz,
                                          angles,shift ,tolerance)
#        print_matrix2D(result,"test_project_-Y")
        for i in range(0,dx):
            for j in range(0,dz):
                self.assertEqual(result[i,dz-1-j],5*(i+j+2))

    def test_project(self):
        """Check projection of a matrix3D for a vector"""
        m1 = create_cube(7,7,9)
        shift = IMP.algebra.Vector3D(0,0,0)
        tolerance = 1e-6
        result = IMP.algebra.Matrix2D()
        ydim=11; xdim=11
        direction = IMP.algebra.Vector3D(1,1,1)
        IMP.em.project_given_direction1(m1,result,ydim,xdim,
                                       direction,shift ,tolerance)
#        print_matrix2D(result,"project_direction")
        opp_result = IMP.algebra.Matrix2D()
        direction = IMP.algebra.Vector3D(-1,-1,-1)
        IMP.em.project_given_direction1(m1,opp_result,ydim,xdim,
                                       direction,shift ,tolerance)
#        print_matrix2D(opp_result,"project_opposite_direction")
        # Check that the two projections mirror each other
        for i in range(0,ydim):
            for j in range(0,xdim):
                # self.assertEqual(result[i,j],opp_result[(ydim-1)-i,j])
                self.assertInTolerance(result[i,j],
                                      opp_result[(ydim-1)-i,j],1e-5)

if __name__ == '__main__':
    unittest.main()
