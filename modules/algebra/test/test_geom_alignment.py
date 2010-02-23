import unittest
import IMP
import IMP.test
import IMP.algebra

class RigidTransformationTests(IMP.test.TestCase):
    """Test rigid transformations"""

    def _produce_point_sets(self, tr):
        vs= IMP.algebra.Vector3Ds()
        vsr= IMP.algebra.Vector3Ds()
        for i in range(0, 20):
            vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
            vsr.append(tr.get_transformed(vs.back()))
        return (vs, vsr)

    def test_align(self):
        """Testing rigid alignment of point sets, no translation"""
        r= IMP.algebra.get_random_rotation_3d()
        t= IMP.algebra.Vector3D(0,0,0) #IMP.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        tr= IMP.algebra.get_transformation_taking_first_to_second(vs, vsr)
        print "tr"
        tr.show()
        print "tr.r"
        tr.get_rotation().show()
        print "tr.t"
        tr.get_translation().show()
        print "r"
        r.show()
        print "t"
        t.show()
        for i in range(0, len(vs)):
            vsr[i].show()
            print
            tr.get_transformed(vs[i]).show()
            print
            print
        q0=tr.get_rotation().get_quaternion()
        q0.show()
        print
        q1=r.get_quaternion()
        q1.show()
        print
        self.assertInTolerance((q0 - q1).get_squared_magnitude(), 0, .1)

    def test_full_align(self):
        """Testing rigid alignment of point sets"""
        r= IMP.algebra.get_random_rotation_3d()
        t= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        tr= IMP.algebra.get_transformation_taking_first_to_second(vs, vsr)
        print "tr"
        tr.show()
        print "tr.r"
        tr.get_rotation().show()
        print "tr.t"
        tr.get_translation().show()
        print "r"
        r.show()
        print "t"
        t.show()
        #for i in range(0, len(vs)):
        #    vsr[i].show()
        #    print
        #    tr.get_transformed(vs[i]).show()
        #    print
        #    print
        self.assertInTolerance((tr.get_rotation().get_quaternion()
                                - r.get_quaternion()).get_squared_magnitude(), 0, .1)


if __name__ == '__main__':
    unittest.main()
