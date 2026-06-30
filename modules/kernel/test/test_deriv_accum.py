import IMP
import IMP.test


class Tests(IMP.test.TestCase):
    """Test DerivativeAccumulator"""

    def test_call(self):
        """Check call of DerivativeAccumulator"""
        da = IMP.DerivativeAccumulator(10.0)
        self.assertAlmostEqual(da(42.0), 420.0, delta=1e-5)

        v = da(IMP.Vector3D(1.0, 3.0, 5.0))
        self.assertIsInstance(v, IMP.Vector3D)
        self.assertSequenceAlmostEqual(v[:], [10.0, 30.0, 50.0],
                                       delta=1e-5)

        v = da(IMP.Vector4D(1.0, 3.0, 5.0, 8.0))
        self.assertIsInstance(v, IMP.Vector4D)
        self.assertSequenceAlmostEqual(v[:], [10.0, 30.0, 50.0, 80.0],
                                       delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
