import IMP
import IMP.test
import IMP.algebra
import pickle


class Tests(IMP.test.TestCase):

    def test_default_constructor(self):
        """Test default constructor"""
        p = IMP.algebra.ConnollySurfacePoint()

    def _assert_equal(self, a, b):
        self.assertEqual(a.get_atom(0), b.get_atom(0))
        self.assertEqual(a.get_atom(1), b.get_atom(1))
        self.assertEqual(a.get_atom(2), b.get_atom(2))
        self.assertAlmostEqual(a.get_area(), b.get_area(), delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(
            a.get_surface_point(), b.get_surface_point()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            a.get_normal(), b.get_normal()), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of ConnollySurfacePoint"""
        p1 = IMP.algebra.ConnollySurfacePoint(1, 2, 3, [4., 5., 6.], 7,
                                              [9., 10., 11.])
        p2 = IMP.algebra.ConnollySurfacePoint(12, 13, 14, [15., 16., 17.], 18,
                                              [19., 20., 21.])
        p2.foo = 'bar'
        dump = pickle.dumps((p1, p2))

        newp1, newp2 = pickle.loads(dump)
        self._assert_equal(p1, newp1)
        self._assert_equal(p2, newp2)
        self.assertEqual(newp2.foo, 'bar')

        self.assertRaises(TypeError, p1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
