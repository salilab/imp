import unittest
import IMP.test
import IMP.algebra as alg
import IMP.em as em
import math
import random


def print_matrix3D(m):
    for i in range(m.get_start(0),m.get_finish(0)+1):
        print "Layer :",i
        for j in range(m.get_start(1),m.get_finish(1)+1):
            for k in range(m.get_start(2),m.get_finish(2)+1):
                print m[i,j,k],
            print "\n"

def print_matrix2D(m):
    r1=range(m.get_start(0),m.get_finish(0)+1)
    r2=range(m.get_start(1),m.get_finish(1)+1)
    for i in r1:
        for j in r2:
            print m[i,j],
        print "\n"


def create_matrix(z,y,x):
    m1 = alg.Matrix3D(z,y,x)
    for i in range(m1.get_start(0),m1.get_finish(0)+1):
        for j in range(m1.get_start(1),m1.get_finish(1)+1):
            for k in range(m1.get_start(2),m1.get_finish(2)+1):
                m1[i,j,k]=i+j+k
#    print_matrix3D(m1)
#    print "==============================================="
    return m1

def create_cube(z,y,x):
    m1 = alg.Matrix3D(z,y,x)
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



class EMprojectTests(IMP.test.TestCase):

    def test_project_Z(self):
        """Check projection of a matrix3D for axis Z"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(0,0,1)
        em.project(m1,result,dy,dx,direction,shift ,tolerance)
        print_matrix2D(result)
        for i in range(0,dy):
            for j in range(0,dx):
                self.assertEqual(result[i,j],3*(i+j+1))

    def test_project_Z_opposite(self):
        """Check projection of a matrix3D for axis -Z"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(0,0,-1)
        em.project(m1,result,dy,dx,direction,shift ,tolerance)
        print_matrix2D(result)
        for i in range(0,dy):
            for j in range(0,dx):
                self.assertEqual(result[(dy-1)-i,(dx-1)-j],3*(i+j+1))


    def test_project_X(self):
        """Check projection of a matrix3D for axis X"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(1,0,0)
        em.project(m1,result,dy,dz,direction,shift ,tolerance)
#        print_matrix2D(result)
        for i in range(0,dy):
            for j in range(0,dz):
                self.assertEqual(result[i,j],9*(i+j+4))


    def test_project_X_opposite(self):
        """Check projection of a matrix3D for axis -X"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(-1,0,0)
        em.project(m1,result,dy,dz,direction,shift ,tolerance)
#        print_matrix2D(result)
        for i in range(0,dy):
            for j in range(0,dz):
                self.assertEqual(result[i,(dz-1)-j],9*(i+j+4))


    def test_project_Y(self):
        """Check projection of a matrix3D for axis Y"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(0,1,0)
        em.project(m1,result,dz,dx,direction,shift ,tolerance)
#        print_matrix2D(result)
        for i in range(0,dz):
            for j in range(0,dx):
                self.assertEqual(result[i,j],5*(i+j+2))


    def test_project_Y_opposite(self):
        """Check projection of a matrix3D for axis -Y"""
        dz=3; dy=5; dx=9
        m1 = create_matrix(dz,dy,dx)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(0,-1,0)
        em.project(m1,result,dz,dx,direction,shift ,tolerance)
#        print_matrix2D(result)
        for i in range(0,dz):
            for j in range(0,dx):
                self.assertEqual(result[(dz-1)-i,j],5*(i+j+2))

    def test_project(self):
        """Check projection of a matrix3D for a vector"""
        m1 = create_cube(7,7,9)
        shift = alg.Vector3D(0,0,0)
        tolerance = 1e-6
        result = alg.Matrix2D()
        direction = alg.Vector3D(1,1,1)
        ydim=11; xdim=11
        em.project(m1,result,ydim,xdim,direction,shift ,tolerance)
#        print_matrix2D(result)
        opp_result = alg.Matrix2D()
        direction = alg.Vector3D(-1,-1,-1)
        em.project(m1,opp_result,ydim,xdim,direction,shift ,tolerance)
#        print_matrix2D(opp_result)
        # Check that the two projections mirror each other
        # (Double mirror in both directions of the projection)
        for i in range(0,ydim):
            for j in range(0,xdim):
                self.assertEqual(result[i,j],opp_result[(xdim-1)-j,(ydim-1)-i])



if __name__ == '__main__':
    unittest.main()
