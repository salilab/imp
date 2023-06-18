import IMP
import IMP.test
import IMP.algebra
import pickle


class Tests(IMP.test.TestCase):
    def test_constructor(self):
        """Test construction of Segment3D objects"""
        V = IMP.algebra.Vector3D
        s = IMP.algebra.Segment3D()
        s = IMP.algebra.Segment3D(V(1, 2, 3), V(4, 5, 6))
        self.assertLess(IMP.algebra.get_distance(s.get_point(0), V(1, 2, 3)),
                        1e-4)
        self.assertLess(IMP.algebra.get_distance(s.get_point(1), V(4, 5, 6)),
                        1e-4)
        self.assertRaisesUsageException(s.get_point, 2)
        self.assertLess(IMP.algebra.get_distance(
            s.get_middle_point(), V(2.5, 3.5, 4.5)), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            s.get_direction(), V(0.57735, 0.57735, 0.57735)), 1e-4)

    def test_projection(self):
        """Test projection onto Segment3D objects"""
        V = IMP.algebra.Vector3D
        s = IMP.algebra.Segment3D(V(0, 0, 0), V(4, 0, 0))
        p = IMP.algebra.get_relative_projection_on_segment(s, V(1, 2, 3))
        self.assertAlmostEqual(p, 0.25, delta=1e-4)

    def test_distance(self):
        """Test distance from Segment3D objects"""
        V = IMP.algebra.Vector3D
        s = IMP.algebra.Segment3D(V(0, 0, 0), V(4, 0, 0))
        self.assertAlmostEqual(IMP.algebra.get_distance(s, V(1, 0, 3)), 3.0,
                               delta=1e-4)
        s2 = IMP.algebra.Segment3D(V(5, 0, 0), V(5, 0, 8))
        self.assertAlmostEqual(IMP.algebra.get_distance(s, s2), 1.0,
                               delta=1e-4)

    def test_namespace_methods(self):
        """Test namespace methods"""
        V = IMP.algebra.Vector3D
        s = IMP.algebra.Segment3D(V(0, 0, 0), V(4, 0, 0))
        bb = IMP.algebra.get_bounding_box(s)
        self.assertLess(IMP.algebra.get_distance(bb.get_corner(0), V(0, 0, 0)),
                        1e-4)
        self.assertLess(IMP.algebra.get_distance(bb.get_corner(1), V(4, 0, 0)),
                        1e-4)
        s2 = IMP.algebra.get_segment_3d_geometry(s)
        self.assertLess(IMP.algebra.get_distance(s2.get_point(0), V(0, 0, 0)),
                        1e-4)
        self.assertLess(IMP.algebra.get_distance(s2.get_point(1), V(4, 0, 0)),
                        1e-4)

    def _assert_equal(self, a, b):
        self.assertLess(IMP.algebra.get_distance(
            a.get_point(0), b.get_point(0)), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            a.get_point(1), b.get_point(1)), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of Segment3D"""
        s1 = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0, 0.0, -4.0),
                                   IMP.algebra.Vector3D(0.0, 0.0, 4.0))
        s2 = IMP.algebra.Segment3D(IMP.algebra.Vector3D(1.0, 2.0, 3.0),
                                   IMP.algebra.Vector3D(0.0, 0.0, 4.0))
        s2.foo = 'bar'
        dump = pickle.dumps((s1, s2))

        news1, news2 = pickle.loads(dump)
        self._assert_equal(s1, news1)
        self._assert_equal(s2, news2)
        self.assertEqual(news2.foo, 'bar')

        self.assertRaises(TypeError, s1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
