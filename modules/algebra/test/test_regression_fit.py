import IMP
import IMP.test
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Test Linear and Parabolic fits"""

    def test_linear(self):
        """Check Linear Fit"""
        v1 = IMP.algebra.Vector2D(1.0, 4.0)
        v2 = IMP.algebra.Vector2D(2.0, 5.0)
        v3 = IMP.algebra.Vector2D(3.0, 6.0)
        vs= []
        vs.append(v1)
        vs.append(v2)
        vs.append(v3)
        lf = IMP.algebra.LinearFit2D(vs)
        lf.show()
        self.assertAlmostEqual(lf.get_a(), 1.0, delta=0.01)
        self.assertAlmostEqual(lf.get_b(), 3.0, delta=0.01)

    def test_weighted_linear(self):
        """Check weighted Linear Fit error"""
        v1 = IMP.algebra.Vector2D(1.0, 4.0)
        v2 = IMP.algebra.Vector2D(2.0, 5.0)
        v3 = IMP.algebra.Vector2D(3.0, 6.0)
        w=[5.0, 1.0, 3.0]
        vs= []
        vs.append(v1)
        vs.append(v2)
        vs.append(v3)
        lf = IMP.algebra.LinearFit2D(vs, w)
        lf.show()
        self.assertAlmostEqual(lf.get_a(), 1.0, delta=0.01)
        self.assertAlmostEqual(lf.get_b(), 3.0, delta=0.01)
        self.assertAlmostEqual(lf.get_fit_error(), 0.0, delta=0.01)

    def test_weighted_linear_sanity(self):
        """Check weighted Linear Fit for sanity"""
        v1 = IMP.algebra.Vector2D(1.0, 4.0)
        v2 = IMP.algebra.Vector2D(2.0, 5.0)
        v3 = IMP.algebra.Vector2D(3.0, 6.0)
        w=[1.0, 1.0, 1.0]
        vs= []
        vs.append(v1)
        vs.append(v2)
        vs.append(v3)
        lf = IMP.algebra.LinearFit2D(vs, w)
        lf.show()
        self.assertAlmostEqual(lf.get_a(), 1.0, delta=0.01)
        self.assertAlmostEqual(lf.get_b(), 3.0, delta=0.01)

    def test_parabolic(self):
        """Check Parabolic Fit"""
        v1 = IMP.algebra.Vector2D(1.0, 9.0)
        v2 = IMP.algebra.Vector2D(2.0, 15.0)
        v3 = IMP.algebra.Vector2D(3.0, 23.0)
        vs= []
        vs.append(v1)
        vs.append(v2)
        vs.append(v3)
        pf = IMP.algebra.ParabolicFit2D(vs)
        pf.show()
        self.assertAlmostEqual(pf.get_a(), 1.0, delta=0.01)
        self.assertAlmostEqual(pf.get_b(), 3.0, delta=0.01)
        self.assertAlmostEqual(pf.get_c(), 5.0, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
