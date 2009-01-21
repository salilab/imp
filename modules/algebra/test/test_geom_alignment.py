import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra

class RigidTransformationTests(IMP.test.TestCase):
    """Test rigid transformations"""

    def _produce_point_sets(self, tr):
        vs= IMP.algebra.Vector3Ds()
        vsr= IMP.algebra.Vector3Ds()
        for i in range(0, 20):
            vs.append(IMP.algebra.random_vector_in_unit_box())
            vsr.append(tr.transform(vs.back()))
        return (vs, vsr)

    def test_align(self):
        """Testing rigid alignment of point sets, no translation"""
        r= IMP.algebra.random_rotation();
        t= IMP.algebra.Vector3D(0,0,0) #IMP.random_vector_in_unit_box()
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        tr= IMP.algebra.rigid_align_first_to_second(vs, vsr)
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
        #    tr.transform(vs[i]).show()
        #    print
        #    print
        self.assertInTolerance((tr.get_rotation().get_quaternion()
                                - r.get_quaternion()).get_squared_magnitude(), 0, .1)

    def test_full_align(self):
        """Testing rigid alignment of point sets"""
        r= IMP.algebra.random_rotation();
        t= IMP.algebra.random_vector_in_unit_box()
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        tr= IMP.algebra.rigid_align_first_to_second(vs, vsr)
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
        #    tr.transform(vs[i]).show()
        #    print
        #    print
        self.assertInTolerance((tr.get_rotation().get_quaternion()
                                - r.get_quaternion()).get_squared_magnitude(), 0, .1)


if __name__ == '__main__':
    unittest.main()
