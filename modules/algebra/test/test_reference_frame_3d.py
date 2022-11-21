import IMP
import IMP.test
import IMP.algebra
import pickle

class Tests(IMP.test.TestCase):
    def test_get_global_ref(self):
        """Test get_global_reference_frame()"""
        T = IMP.algebra.Transformation3D
        V = IMP.algebra.Vector3D
        idrot = IMP.algebra.get_identity_rotation_3d()
        r1 = IMP.algebra.ReferenceFrame3D(T(idrot, V(1,2,3)))
        r2 = IMP.algebra.ReferenceFrame3D(T(idrot, V(4,5,6)))
        r3 = r1.get_global_reference_frame(r2)
        v3 = r3.get_transformation_to().get_translation()
        self.assertLess(IMP.algebra.get_distance(v3, V(5,7,9)), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of ReferenceFrame3D"""
        t1 = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
        t2 = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
        rf1 = IMP.algebra.ReferenceFrame3D(t1)
        rf2 = IMP.algebra.ReferenceFrame3D(t2)
        rf2.foo = 'bar'
        dump = pickle.dumps((rf1, rf2))

        newrf1, newrf2 = pickle.loads(dump)
        self._assert_equal(rf1, newrf1)
        self._assert_equal(rf2, newrf2)
        self.assertEqual(newrf2.foo, 'bar')

    def _assert_equal(self, a, b):
        ta = a.get_transformation_to()
        tb = b.get_transformation_to()
        self.assertLess(
            IMP.algebra.get_distance(ta.get_rotation(),
                                     tb.get_rotation()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(ta.get_translation(),
                                     tb.get_translation()), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
