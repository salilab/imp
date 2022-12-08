import IMP
import IMP.test
import IMP.algebra
import pickle


class Tests(IMP.test.TestCase):

    """Test Linear and Parabolic fits"""

    def test_linear(self):
        """Check Linear Fit"""
        default = IMP.algebra.LinearFit2D()
        v1 = IMP.algebra.Vector2D(1.0, 4.0)
        v2 = IMP.algebra.Vector2D(2.0, 5.0)
        v3 = IMP.algebra.Vector2D(3.0, 6.0)
        vs = []
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
        w = [5.0, 1.0, 3.0]
        vs = []
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
        w = [1.0, 1.0, 1.0]
        vs = []
        vs.append(v1)
        vs.append(v2)
        vs.append(v3)
        lf = IMP.algebra.LinearFit2D(vs, w)
        lf.show()
        self.assertAlmostEqual(lf.get_a(), 1.0, delta=0.01)
        self.assertAlmostEqual(lf.get_b(), 3.0, delta=0.01)

    def test_parabolic(self):
        """Check Parabolic Fit"""
        default = IMP.algebra.ParabolicFit2D()
        v1 = IMP.algebra.Vector2D(1.0, 9.0)
        v2 = IMP.algebra.Vector2D(2.0, 15.0)
        v3 = IMP.algebra.Vector2D(3.0, 23.0)
        vs = []
        vs.append(v1)
        vs.append(v2)
        vs.append(v3)
        pf = IMP.algebra.ParabolicFit2D(vs)
        pf.show()
        self.assertAlmostEqual(pf.get_a(), 1.0, delta=0.01)
        self.assertAlmostEqual(pf.get_b(), 3.0, delta=0.01)
        self.assertAlmostEqual(pf.get_c(), 5.0, delta=0.01)
        self.assertAlmostEqual(pf.get_fit_error(), 0.0, delta=0.01)

    def test_linear_fit_pickle(self):
        """Test (un-)pickle of LinearFit2D"""
        lf1 = IMP.algebra.LinearFit2D([IMP.algebra.Vector2D(1.0, 4.0),
                                       IMP.algebra.Vector2D(2.0, 5.0),
                                       IMP.algebra.Vector2D(3.0, 6.0)])
        lf2 = IMP.algebra.LinearFit2D([IMP.algebra.Vector2D(1.0, 4.0),
                                       IMP.algebra.Vector2D(2.0, 6.0),
                                       IMP.algebra.Vector2D(3.0, 8.0)])
        lf2.foo = 'bar'
        dump = pickle.dumps((lf1, lf2))

        newlf1, newlf2 = pickle.loads(dump)
        self._assert_lf_equal(lf1, newlf1)
        self._assert_lf_equal(lf2, newlf2)
        self.assertEqual(newlf2.foo, 'bar')

        self.assertRaises(TypeError, lf1._set_from_binary, 42)

    def _assert_lf_equal(self, a, b):
        self.assertAlmostEqual(a.get_a(), b.get_a(), delta=1e-4)
        self.assertAlmostEqual(a.get_b(), b.get_b(), delta=1e-4)
        self.assertAlmostEqual(a.get_fit_error(), b.get_fit_error(),
                               delta=1e-4)

    def test_parabolic_fit_pickle(self):
        """Test (un-)pickle of ParabolicFit2D"""
        pf1 = IMP.algebra.ParabolicFit2D([IMP.algebra.Vector2D(1.0, 4.0),
                                          IMP.algebra.Vector2D(2.0, 5.0),
                                          IMP.algebra.Vector2D(3.0, 6.0)])
        pf2 = IMP.algebra.ParabolicFit2D([IMP.algebra.Vector2D(1.0, 4.0),
                                          IMP.algebra.Vector2D(2.0, 6.0),
                                          IMP.algebra.Vector2D(3.0, 8.0)])
        pf2.foo = 'bar'
        dump = pickle.dumps((pf1, pf2))

        newpf1, newpf2 = pickle.loads(dump)
        self._assert_pf_equal(pf1, newpf1)
        self._assert_pf_equal(pf2, newpf2)
        self.assertEqual(newpf2.foo, 'bar')

        self.assertRaises(TypeError, pf1._set_from_binary, 42)

    def _assert_pf_equal(self, a, b):
        self.assertAlmostEqual(a.get_a(), b.get_a(), delta=1e-4)
        self.assertAlmostEqual(a.get_b(), b.get_b(), delta=1e-4)
        self.assertAlmostEqual(a.get_c(), b.get_c(), delta=1e-4)
        self.assertAlmostEqual(a.get_fit_error(), b.get_fit_error(),
                               delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
