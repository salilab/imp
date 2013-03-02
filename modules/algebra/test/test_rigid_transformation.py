import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):
    """Test rigid transformations"""

    def setUp(self):
        """Build a set of test vectors"""
        IMP.test.TestCase.setUp(self)
        self.v1 = IMP.algebra.Vector3D(-63.537,76.945,84.162)
        self.v2 = IMP.algebra.Vector3D(-41.472,8.922,-63.657)


    def test_transformation(self):
        """Check that the rotation function is ok"""
        rt = IMP.algebra.get_rotation_from_fixed_xyz(0.2,0.8,-0.4)
        t=IMP.algebra.Transformation3D(rt,IMP.algebra.Vector3D(20.0,-12.4,18.6))
        v1_t = t.get_transformed(self.v1)
        v2_t = t.get_transformed(self.v2)
        v1_t_res=IMP.algebra.Vector3D(-62.517,86.209, 41.139)
        v2_t_res=IMP.algebra.Vector3D( 41.767, 1.621,-53.381)
        self.assertEqual((v1_t- v1_t_res).get_magnitude() < 0.01,True)
        self.assertEqual((v2_t- v2_t_res).get_magnitude() < 0.01,True)

    def test_delta(self):
        """Check that the delta transformation between two transformations is correct"""
        rot1=IMP.algebra.get_random_rotation_3d()
        trans1 = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-10.,-10.,-10),
                                                  IMP.algebra.Vector3D(10.,10.,10)))
        rot2=IMP.algebra.get_random_rotation_3d()
        trans2 = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-10.,-10.,-10),
                                                  IMP.algebra.Vector3D(10.,10.,10)))
        t1 = IMP.algebra.Transformation3D(rot1,trans1)
        t2 = IMP.algebra.Transformation3D(rot2,trans2)
        #a transformation to apply on rot1 to get to rot2
        t12 =  t2/t1
        #a transformation to apply on rot2 to get to rot1
        t21 =  t1/t2
        t22 = t12*t1
        #check that rot22 is indeed rot2
        id_rot = (t22.get_inverse()*t2).get_rotation()
        q_id = id_rot.get_quaternion()
        self.assertAlmostEqual(q_id[0],1.0)
        self.assertAlmostEqual(q_id[1],0.0)
        self.assertAlmostEqual(q_id[2],0.0)
        self.assertAlmostEqual(q_id[3],0.0)

        t11 = t21*t2
        #check that rot11 is indeed rot1
        id_rot = (t11.get_inverse()*t1).get_rotation()
        q_id = id_rot.get_quaternion()
        self.assertAlmostEqual(q_id[0],1.0)
        self.assertAlmostEqual(q_id[1],0.0)
        self.assertAlmostEqual(q_id[2],0.0)
        self.assertAlmostEqual(q_id[3],0.0)

if __name__ == '__main__':
    IMP.test.main()
