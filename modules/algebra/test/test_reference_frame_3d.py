import IMP
import IMP.test
import IMP.algebra

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

if __name__ == '__main__':
    IMP.test.main()
