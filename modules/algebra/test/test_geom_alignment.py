import IMP
import IMP.test
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Test rigid transformations"""

    def _produce_point_sets(self, tr):
        vs= []
        vsr= []
        for i in range(0, 20):
            vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
            print vs[-1]
            vsr.append(tr.get_transformed(vs[-1]))
            print vsr[-1]
        return (vs, vsr)

    def test_align(self):
        """Testing rigid alignment of point sets, no translation"""
        r= IMP.algebra.get_random_rotation_3d()
        t= IMP.algebra.Vector3D(0,0,0) #IMP.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        tr= IMP.algebra.get_transformation_aligning_first_to_second(vs, vsr)
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
            print i
            vsr[i].show()
            print
            tr.get_transformed(vs[i]).show()
            print
            print
        print "getting quaternion"
        q0=tr.get_rotation().get_quaternion()
        print "it is ", q0
        print
        q1=r.get_quaternion()
        print "other is ", q1
        print
        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_rotation(), r),
                               0, delta=.1)
        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_translation(),
                                                        t),
                               0, delta=.1)

    def test_full_align(self):
        """Testing rigid alignment of point sets"""
        r= IMP.algebra.get_random_rotation_3d()
        t= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        tr= IMP.algebra.get_transformation_aligning_first_to_second(vs, vsr)
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
        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_rotation(), r),
                               0, delta=.1)
        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_translation(),
                                                        t),
                               0, delta=.1)


if __name__ == '__main__':
    IMP.test.main()
