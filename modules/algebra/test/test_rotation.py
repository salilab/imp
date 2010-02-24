import unittest
import IMP
import IMP.test
import IMP.algebra
import random
import math

class TransformFunct:
    def __init__(self, x_index, q_index, x_base, q_base):
        self.xi= x_index
        self.x= x_base
        self.qi= q_index
        self.q= q_base
        #print self.xi
        #print self.qi
        #self.x.show(); print
        #print self.q
    def __call__(self, v):
        self.q[self.qi]= v
        uv= self.q.get_unit_vector()
        r= IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        rx= r.get_rotated(self.x)
        return rx[self.xi]
    def get_analytic_deriv(self):
        uv= self.q.get_unit_vector()
        r= IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        return r.get_derivative(self.x, self.qi)[self.xi]

class RotationTests(IMP.test.TestCase):
    """Test rotations"""

    def test_axis_rotation(self):
        """Check rotation about axis """
        for ii in xrange(1,10):
            axis= IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
            angle= math.pi/ii#random.uniform(-math.pi,math.pi)

            r= IMP.algebra.get_rotation_in_radians_about_axis(axis,angle)
            ri= r.get_inverse()
            v_start = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
            vt = v_start
            for i in xrange(2*ii):
                vt= r.get_rotated(vt)
                if i==0:
                    vti= ri.get_rotated(vt)
            self.assertInTolerance(IMP.algebra.get_distance(v_start,vt),0., .1)
            self.assertInTolerance(vti[0], v_start[0], .1)
            self.assertInTolerance(vti[1], v_start[1], .1)
            self.assertInTolerance(vti[2], v_start[2], .1)




    def test_rotation(self):
        """Check that the rotation inverse is an inverse"""
        axis= IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
        m= random.uniform(-1,1)
        mag= m*m+ axis*axis
        mag = mag**(.5)
        axis/= mag
        m/=mag
        r= IMP.algebra.Rotation3D(m, axis[0], axis[1], axis[2])
        ri= r.get_inverse()
        v= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        vt= r.get_rotated(v)
        vti= ri.get_rotated(vt)
        v.show()
        vti.show()
        vt.show()
        self.assertInTolerance(vti[0], v[0], .1)
        self.assertInTolerance(vti[1], v[1], .1)
        self.assertInTolerance(vti[2], v[2], .1)

    def test_deriv(self):
        """Check the quaternion derivatives"""
        #r= IMP.algebra.Rotation3D(1,0,0,0)
        r=IMP.algebra.get_random_rotation_3d()
        x= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        #x=IMP.algebra.Vector3D(1,0,0)
        for qi in range(0,4):
            for xi in range(0,3):
                print "qi="+str(qi) +" and xi= " +str(xi)
                tf= TransformFunct(xi, qi, x, r.get_quaternion())
                d=IMP.test.numerical_derivative(tf, r.get_quaternion()[qi], .01)
                print d
                ad= tf.get_analytic_deriv()
                print ad
                self.assertInTolerance(d, ad, .05)

    def test_rotation_between_vectors(self):
        """Check that the rotation between two vectors is correct"""
        axis=[]
        axis.append(IMP.algebra.Vector3D(1.0,0.0,0.0))
        axis.append(IMP.algebra.Vector3D(0.0,1.0,0.0))
        axis.append(IMP.algebra.Vector3D(0.0,1.0,0.0)) #to check the parallel case
        axis.append(IMP.algebra.Vector3D(0.0,0.0,1.0))
        axis.append(IMP.algebra.Vector3D(0.0,0.0,-1.0))#to check the antiparallel case
        axis.append(IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d()))
        axis.append(IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d()))
        for i in range(len(axis)-1):
            rot = IMP.algebra.get_rotation_taking_first_to_second(axis[i],axis[i+1])
            self.assertAlmostEqual(IMP.algebra.get_distance(rot*axis[i],axis[i+1]),0.0)

    def test_decompose_rotation_into_axis_angle(self):
        """Check that the function decompose_rotation_into_axis_angle is correct"""
        r0= IMP.algebra.get_random_rotation_3d()
        aa = IMP.algebra.decompose_rotation_into_axis_angle(r0)
        axis=aa[0];angle=aa[1]
        r1 = IMP.algebra.get_rotation_in_radians_about_axis(axis,angle)
        self.assertInTolerance((r0.get_quaternion()-r1.get_quaternion()).get_squared_magnitude(), 0, .1)

    def test_is_equal_between_rotations(self):
        """Check that two rotations are equal"""
        t1 = IMP.algebra.get_random_rotation_3d()
        t2 = t1
        t3 = t1.get_inverse()
        self.assert_(IMP.algebra.almost_equal_rotations(t1,t2)==True)
        self.assert_(IMP.algebra.almost_equal_rotations(t1,t3)==False)

    def test_interpolate(self):
        """Check that rotations can be interpolated"""
        r0= IMP.algebra.get_random_rotation_3d()
        r1= IMP.algebra.get_random_rotation_3d()
        print "Inputs"
        r0.show()
        r1.show()
        r5= IMP.algebra.interpolate(r0, r1, .5)
        d= r5/r0
        r1p= d*d*r0
        print "final"
        r1.show()
        print "out"
        r1p.show()
        self.assertInTolerance((r1.get_quaternion()-r1p.get_quaternion()).get_squared_magnitude(), 0, .1)

if __name__ == '__main__':
    unittest.main()
