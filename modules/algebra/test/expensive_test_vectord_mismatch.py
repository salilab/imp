import IMP.test


class Tests(IMP.test.TestCase):

    def test_vectord_mismatch(self):
        """Test combination of VectorD of different dimensions"""
        # Should be a compile-time error to combine VectorD of different sizes
        self.assertCompileFails(
            headers=['IMP/algebra/VectorD.h'],
            body="""
IMP::algebra::Vector3D v1(1,2,3);
IMP::algebra::Vector4D v3(7,8,9,10);
v1 -= v3;""")


if __name__ == '__main__':
    IMP.test.main()
