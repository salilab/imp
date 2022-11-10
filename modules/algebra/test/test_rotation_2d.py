import IMP
import IMP.test
import IMP.algebra
import math
import io
import pickle

class Tests(IMP.test.TestCase):

    def test_get_random(self):
        """Check get_random_rotation_2d()"""
        r = IMP.algebra.get_random_rotation_2d()
        angle = r.get_angle()
        self.assertLess(angle, math.pi * 2.00001)
        self.assertGreaterEqual(angle, 0.)

    def test_get_rotated_vector(self):
        """Test get_rotated() with a vector"""
        dr = IMP.algebra.Rotation2D()
        r = IMP.algebra.Rotation2D(math.pi / 2.)
        self.assertRaisesInternalException(dr.get_rotated,
                                           IMP.algebra.Vector2D(1,2))
        v = r.get_rotated(IMP.algebra.Vector2D(1,2))
        self.assertLess(IMP.algebra.get_distance(v, IMP.algebra.Vector2D(-2,1)),
                        1e-4)

    def test_get_rotated_point(self):
        """Test get_rotated() with a point"""
        dr = IMP.algebra.Rotation2D()
        r = IMP.algebra.Rotation2D(math.pi / 2.)
        self.assertRaisesInternalException(dr.get_rotated, 1, 2)
        v = r.get_rotated(1, 2)
        self.assertLess(IMP.algebra.get_distance(v, IMP.algebra.Vector2D(-2,1)),
                        1e-4)

    def test_get_inverse(self):
        """Test get_inverse()"""
        dr = IMP.algebra.Rotation2D()
        r = IMP.algebra.Rotation2D(math.pi / 2.)
        self.assertRaisesInternalException(dr.get_inverse)
        r2 = r.get_inverse()
        self.assertAlmostEqual(r2.get_angle(), -math.pi / 2., delta=1e-4)

    def test_angle(self):
        """Test get/set angle"""
        r = IMP.algebra.Rotation2D()
        r.set_angle(math.pi / 2.)
        self.assertAlmostEqual(r.get_angle(), math.pi / 2., delta=1e-4)

    def test_showable(self):
        """Test showable"""
        r = IMP.algebra.Rotation2D(math.pi / 2.)
        sio = io.BytesIO()
        r.show(sio)

    def test_pickle(self):
        """Check that rotations can be (un-)pickled"""
        r1 = IMP.algebra.get_random_rotation_2d()
        r2 = IMP.algebra.get_random_rotation_2d()
        r2.foo = 'bar'
        rdump = pickle.dumps((r1, r2))

        newr1, newr2 = pickle.loads(rdump)
        self.assertAlmostEqual(newr1.get_angle(), r1.get_angle(), delta=1e-4)
        self.assertAlmostEqual(newr2.get_angle(), r2.get_angle(), delta=1e-4)
        self.assertEqual(newr2.foo, 'bar')

        # Make sure that sin/cos were correctly restored
        v = r1.get_rotated(1, 2)
        newv = newr1.get_rotated(1, 2)
        self.assertLess(IMP.algebra.get_distance(v, newv), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
