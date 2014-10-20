import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

    def test_trivial_constructor(self):
        """Test trivial Transformation2D constructor"""
        t = IMP.algebra.Transformation2D()

    def test_identity(self):
        """Test Transformation2D identity"""
        t = IMP.algebra.get_identity_transformation_2d()
        self.assertAlmostEqual(t.get_rotation().get_angle(), 0., delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(t.get_translation(),
                                            IMP.algebra.Vector2D(0,0)), 1e-4)

    def test_rotate_about_point(self):
        """Test rotation about a 2D point"""
        p = IMP.algebra.Vector2D(1., 2.)
        r = IMP.algebra.Rotation2D(math.pi / 2.)
        t = IMP.algebra.get_rotation_about_point(p, r)
        n = t * IMP.algebra.Vector2D(2., 3.)
        self.assertLess(IMP.algebra.get_distance(n,
                                    IMP.algebra.Vector2D(0,3)), 1e-4)

    def test_inverse(self):
        """Test inverse of Transformation2D"""
        t = IMP.algebra.Transformation2D(IMP.algebra.Rotation2D(math.pi / 2.),
                                          IMP.algebra.Vector2D(1,2))
        t2 = t.get_inverse()
        self.assertAlmostEqual(t2.get_rotation().get_angle(), -math.pi / 2.,
                               delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(t2.get_translation(),
                                            IMP.algebra.Vector2D(-2,1)), 1e-4)
        t2 = t * t.get_inverse()
        self.assertAlmostEqual(t2.get_rotation().get_angle(), 0., delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(t2.get_translation(),
                                            IMP.algebra.Vector2D(0,0)), 1e-4)

    def test_compose(self):
        """Test compose of Transformation2Ds"""
        t = IMP.algebra.Transformation2D(IMP.algebra.Rotation2D(math.pi / 2.),
                                          IMP.algebra.Vector2D(1,2))
        t2 = IMP.algebra.Transformation2D(IMP.algebra.Rotation2D(math.pi / 2.),
                                          IMP.algebra.Vector2D(4,3))
        t3 = IMP.algebra.compose(t, t2)
        self.assertAlmostEqual(t3.get_rotation().get_angle(), math.pi,
                               delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(t3.get_translation(),
                                            IMP.algebra.Vector2D(-2,6)), 1e-4)

    def test_operations(self):
        """Test operations on a Transformation2D"""
        v = IMP.algebra.Vector2D(1, 2)
        r = IMP.algebra.Rotation2D(math.pi / 2.)
        t = IMP.algebra.Transformation2D(r, v)
        t.show()
        t2 = IMP.algebra.Transformation2D(IMP.algebra.Rotation2D(math.pi / 2.),
                                          IMP.algebra.Vector2D(4,3))
        o = t.get_transformed(IMP.algebra.Vector2D(3,4))
        self.assertLess(IMP.algebra.get_distance(o,
                                    IMP.algebra.Vector2D(-3,5)), 1e-4)
        o = t * IMP.algebra.Vector2D(3,4)
        self.assertLess(IMP.algebra.get_distance(o,
                                    IMP.algebra.Vector2D(-3,5)), 1e-4)
        tt2 = t * t2
        self.assertAlmostEqual(tt2.get_rotation().get_angle(), math.pi,
                               delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(tt2.get_translation(),
                                            IMP.algebra.Vector2D(-2,6)), 1e-4)
        t *= t2
        self.assertAlmostEqual(t.get_rotation().get_angle(), math.pi,
                               delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(t.get_translation(),
                                            IMP.algebra.Vector2D(-2,6)), 1e-4)
        t = IMP.algebra.Transformation2D(r, v)
        tt2 = t/t2
        self.assertAlmostEqual(tt2.get_rotation().get_angle(), 0., delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(tt2.get_translation(),
                                            IMP.algebra.Vector2D(-3,-1)), 1e-4)
        t /= t2
        self.assertAlmostEqual(t.get_rotation().get_angle(), 0., delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(t.get_translation(),
                                            IMP.algebra.Vector2D(-3,-1)), 1e-4)


    def test_transformation_from_point_sets(self):
        """Check building a Transformation2D from point sets"""
        x1 = IMP.algebra.Vector2D(1, 2)
        x2 = IMP.algebra.Vector2D(6, 8)
        angle_applied = math.pi / 4.
        shift_applied = IMP.algebra.Vector2D(-2, 4)
        R = IMP.algebra.Rotation2D(angle_applied)
        y1 = R.get_rotated(x1) + shift_applied
        y2 = R.get_rotated(x2) + shift_applied
        set1 = [x1, x2]
        set2 = [y1, y2]
        T = IMP.algebra.get_transformation_aligning_pair(set1, set2)
        self.assertAlmostEqual(angle_applied, T.get_rotation().get_angle(),
                               delta=.01)
        self.assertAlmostEqual(shift_applied[0], T.get_translation()[0],
                               delta=.01)
        self.assertAlmostEqual(shift_applied[1], T.get_translation()[1],
                               delta=.01)

if __name__ == '__main__':
    IMP.test.main()
