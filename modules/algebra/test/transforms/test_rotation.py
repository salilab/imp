import unittest
import IMP
import IMP.utils
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
        rx= r.rotate(self.x)
        return rx[self.xi]
    def get_analytic_deriv(self):
        uv= self.q.get_unit_vector()
        r= IMP.algebra.Rotation3D(uv[0], uv[1], uv[2], uv[3])
        return r.get_derivative(self.x, self.qi)[self.xi]

class RigidTransformationTests(IMP.test.TestCase):
    """Test particles"""

    def _test_rotation(self):
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

    def test_deriv(self):
        """Check the quaternion derivatives"""
        r= IMP.algebra.Rotation3D(1,0,0,0)#IMP.algebra.random_rotation()
        #x= IMP.algebra.random_vector_in_unit_box()
        x=IMP.algebra.Vector3D(1,0,0)
        for qi in range(0,4):
            for xi in range(0,3):
                print "qi="+str(qi) +" and xi= " +str(xi)
                tf= TransformFunct(xi, qi, x, r.get_quaternion())
                d=IMP.test.numerical_derivative(tf, r.get_quaternion()[qi], .01)
                print d
                ad= tf.get_analytic_deriv()
                print ad
                self.assertInTolerance(d, ad, .05)

if __name__ == '__main__':
    unittest.main()
