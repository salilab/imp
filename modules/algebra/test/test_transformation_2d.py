import IMP
import IMP.test
import IMP.algebra
import math


class Tests(IMP.test.TestCase):

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
