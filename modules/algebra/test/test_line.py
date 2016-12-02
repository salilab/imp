import math

import IMP
import IMP.test
import IMP.algebra
from IMP.algebra import Vector3D as V3


class Tests(IMP.test.TestCase):

    """Tests for Line3D objects and related functions"""

    def test_constructor(self):
        """Test construction of Line3D objects"""
        l = IMP.algebra.Line3D()
        l = IMP.algebra.Line3D(IMP.algebra.Segment3D(V3(0, 0, 0), V3(1, 2, 3)))
        self.assertAlmostEqual(
            IMP.algebra.get_distance(l.get_direction(),
                                     V3(1, 2, 3).get_unit_vector()), 0,
            delta=1e-6)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(l.get_point_on_line(), V3(0, 0, 0)), 0,
            delta=1e-6)
        s = l.get_segment_starting_at(V3(0, 0, 0), V3(1, 2, 3).get_magnitude())
        self.assertAlmostEqual(IMP.algebra.get_distance(s.get_point(0),
                                                        V3(0, 0, 0)), 0,
                               delta=1e-6)
        self.assertAlmostEqual(IMP.algebra.get_distance(s.get_point(1),
                                                        V3(1, 2, 3)), 0,
                               delta=1e-6)
        l = IMP.algebra.Line3D(V3(0, 0, 1), V3(1, 2, 3))
        self.assertAlmostEqual(IMP.algebra.get_distance(l.get_direction(),
                                                        V3(0, 0, 1)), 0,
                               delta=1e-6)
        self.assertAlmostEqual(IMP.algebra.get_distance(l.get_point_on_line(),
                                                        V3(1, 2, 0)), 0,
                               delta=1e-6)
        self.assertAlmostEqual(
            l.get_moment().get_scalar_product(l.get_point_on_line()), 0,
            delta=1e-6)
        self.assertAlmostEqual(
            l.get_moment().get_scalar_product(l.get_direction()), 0,
            delta=1e-6)
        self.assertAlmostEqual(l.get_moment().get_magnitude(),
                               l.get_point_on_line().get_magnitude(),
                               delta=1e-6)
        self.assertAlmostEqual((l.get_direction() +
                                l.get_opposite().get_direction()
                                ).get_magnitude(), 0, delta=1e-6)

    def test_projection(self):
        """Test projection onto Line3D objects"""
        l = IMP.algebra.Line3D(V3(1, 0, 0), V3(0, 0, 0))
        p = IMP.algebra.get_projected(l, V3(1, 2, 3))
        self.assertAlmostEqual(p.get_magnitude(), 1, delta=1e-6)
        s1 = IMP.algebra.Segment3D(V3(-3, 1, 2), V3(.5, 3, 4))
        s2 = IMP.algebra.get_projected(l, s1)
        self.assertAlmostEqual(IMP.algebra.get_distance(s2.get_point(0),
                                                        V3(-3, 0, 0)), 0,
                               delta=1e-6)
        self.assertAlmostEqual(IMP.algebra.get_distance(s2.get_point(1),
                                                        V3(.5, 0, 0)), 0,
                               delta=1e-6)

    def test_distance(self):
        """Test distance from Line3D objects"""
        l = IMP.algebra.Line3D(V3(1, 0, 0), V3(0, 0, 0))
        self.assertAlmostEqual(IMP.algebra.get_distance(l, V3(1, 0, 3)), 3.0,
                               delta=1e-6)
        l2 = IMP.algebra.Line3D(V3(0, 0, 1), V3(0, 5, 0))
        self.assertAlmostEqual(
            IMP.algebra.get_segment_connecting_first_to_second(
                l, l2).get_length(), 5.,
            delta=1e-6)
        self.assertAlmostEqual(IMP.algebra.get_distance(l, l2), 5., delta=1e-6)
        l2 = IMP.algebra.Line3D(V3(1, 0, 0), V3(0, 6, 0))
        self.assertAlmostEqual(IMP.algebra.get_distance(l, l2), 6., delta=1e-6)

    def test_angle(self):
        """Test angle from Line3D objects"""
        l = IMP.algebra.Line3D(V3(1, 0, 0), V3(0, 0, 0))
        self.assertAlmostEqual(IMP.algebra.get_distance(l, V3(1, 0, 3)), 3.0,
                               delta=1e-6)
        l2 = IMP.algebra.Line3D(V3(0, 0, 1), V3(0, 5, 0))
        self.assertAlmostEqual(IMP.algebra.get_angle(l, l2), math.pi / 2.,
                               delta=1e-6)
        l2 = IMP.algebra.Line3D(V3(-1, 0, 0), V3(0, 5, 0))
        self.assertAlmostEqual(IMP.algebra.get_angle(l, l2), math.pi,
                               delta=1e-6)
        l2 = IMP.algebra.Line3D(V3(0, 1, 0), V3(0, 5, 0))
        self.assertAlmostEqual(IMP.algebra.get_angle(l, l2), math.pi / 2.,
                               delta=1e-6)

    def test_connecting_segments(self):
        """Test segments connecting Line3D objects."""
        l = IMP.algebra.Line3D(V3(1, 0, 0), V3(1, 0, 0))
        l2 = IMP.algebra.Line3D(V3(0, 1, 0), V3(0, 1, 1))
        s = IMP.algebra.get_segment_connecting_first_to_second(l, l2)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(s.get_point(0), V3(0, 0, 0)), 0.,
            delta=1e-6)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(s.get_point(1), V3(0, 0, 1)), 0.,
            delta=1e-6)
        l2 = IMP.algebra.Line3D(V3(1, 0, 0), V3(1, 5, 0))
        s = IMP.algebra.get_segment_connecting_first_to_second(l, l2)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(s.get_point(0), V3(0, 0, 0)), 0.,
            delta=1e-6)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(s.get_point(1), V3(0, 5, 0)), 0.,
            delta=1e-6)

    def test_namespace_methods(self):
        s = IMP.algebra.Line3D(V3(1, 0, 0), V3(4, 0, 0))
        bb = IMP.algebra.get_bounding_box(s)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(bb.get_corner(0), V3(0, 0, 0)), 0,
            delta=1e-6)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(bb.get_corner(1), V3(1, 0, 0)), 0,
            delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
