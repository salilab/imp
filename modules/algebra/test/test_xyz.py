import unittest
import IMP
import IMP.test
import IMP.algebra
import math
import random

class XYZTests(IMP.test.TestCase):
    def test_cylinder_construction(self):
        """Check Euler XYZ conversions"""
        x= random.uniform(0,math.pi)
        y= random.uniform(0,math.pi)
        z= random.uniform(0,math.pi)
        print "In"
        print x
        print y
        print z
        IMP.set_log_level(IMP.VERBOSE)
        r= IMP.algebra.rotation_from_fixed_xyz(x,y,z)
        e= IMP.algebra.fixed_xyz_from_rotation(r)
        print "Out"
        print e.get_x()
        print e.get_y()
        print e.get_z()
        #we can not compare x to e.get_x() as tehy can be PI shifted.
        r2 = IMP.algebra.rotation_from_fixed_xyz(e.get_x(),e.get_y(),e.get_z())
        id_mat = r*r2.get_inverse()
        q = id_mat.get_quaternion()
        #check that the composed matrix is the identity matrix.
        self.assertInTolerance(q[0],1.,.1)
        self.assertInTolerance(q[1],0.,.1)
        self.assertInTolerance(q[2],0.,.1)
        self.assertInTolerance(q[3],0.,.1)

if __name__ == '__main__':
    unittest.main()
