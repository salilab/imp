import IMP
import IMP.test
import IMP.algebra
import math
import StringIO

class Tests(IMP.test.TestCase):

    def test_empty_constructor(self):
        """Test creation of empty KD-bounding box"""
        b3 = IMP.algebra.BoundingBoxKD(3)
        b4 = IMP.algebra.BoundingBoxKD(4)

    def test_bounding_box_construction(self):
        """Check that the bounding box is constructed correctly"""
        points = []
        bound1 = IMP.algebra.Vector3D(0.0, 10.0, 0.0)
        bound2 = IMP.algebra.Vector3D(10.0, 0.0, 10.0)
        points.append(bound1)
        points.append(bound2)
        small_box = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1, 1, 1),
                                              IMP.algebra.Vector3D(9, 9, 9))
        self.assertAlmostEqual(IMP.algebra.get_volume(small_box), 512.,
                               delta=.1)
        for i in range(0, 20):
            v = IMP.algebra.get_random_vector_in(small_box)
            points.append(v)
        bb = IMP.algebra.BoundingBox3D(points)
        self.assertAlmostEqual(IMP.algebra.get_distance(
            bb.get_corner(0), IMP.algebra.Vector3D(0, 0, 0)), 0.001, places=1)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(bb.get_corner(1),
                                     IMP.algebra.Vector3D(10,
                                                          10,
                                                          10)),
            0.001,
            places=1)

    def test_bounding_box_union(self):
        """Check the union function"""
        b1 = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1, 1, 1),
                                       IMP.algebra.Vector3D(9, 9, 9))
        b2 = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5, -5, -5),
                                       IMP.algebra.Vector3D(3, 3, 3))
        bup = b1 + b2
        bu = IMP.algebra.get_union(b1, b2)
        self.assertAlmostEqual(IMP.algebra.get_distance(bu.get_corner(0),
                                                        IMP.algebra.Vector3D(-5, -5, -5)), 0.001, places=1)
        self.assertAlmostEqual(IMP.algebra.get_distance(bu.get_corner(1),
                                                        IMP.algebra.Vector3D(9, 9, 9)), 0.001, places=1)

    def test_addition_bb(self):
        """Check BoundingBox3D addition of BoundingBox3D"""
        V = IMP.algebra.Vector3D
        b1 = IMP.algebra.BoundingBox3D(V(1, 1, 1), V(9, 9, 9))
        b2 = IMP.algebra.BoundingBox3D(V(-5, -5, -5), V(3, 3, 3))
        idb1 = id(b1)
        cppobj = str(b1.this)
        bsum = b1 + b2
        b1 += b2
        # Inplace addition should not change the Python object identity:
        self.assertEqual(id(b1), idb1)
        # The underlying C++ object pointer should be unchanged too:
        self.assertEqual(str(b1.this), cppobj)
        for obj in b1, bsum:
            self.assertLess(IMP.algebra.get_distance(b1.get_corner(0),
                                                     V(-5, -5, -5)), 1e-4)
            self.assertLess(IMP.algebra.get_distance(b1.get_corner(1),
                                                     V(9, 9, 9)), 1e-4)

    def test_addition_vector(self):
        """Check BoundingBox3D addition of Vector3D"""
        V = IMP.algebra.Vector3D
        b1 = IMP.algebra.BoundingBox3D(V(1, 1, 1), V(9, 9, 9))
        v2 = V(-5, -5, -5)
        idb1 = id(b1)
        cppobj = str(b1.this)
        bsum = b1 + v2
        b1 += v2
        # Inplace addition should not change the Python object identity:
        self.assertEqual(id(b1), idb1)
        # The underlying C++ object pointer should be unchanged too:
        self.assertEqual(str(b1.this), cppobj)
        for obj in b1, bsum:
            self.assertLess(IMP.algebra.get_distance(b1.get_corner(0),
                                                     V(-5, -5, -5)), 1e-4)
            self.assertLess(IMP.algebra.get_distance(b1.get_corner(1),
                                                     V(9, 9, 9)), 1e-4)

    def test_addition_float(self):
        """Check BoundingBox3D addition of a float"""
        V = IMP.algebra.Vector3D
        b1 = IMP.algebra.BoundingBox3D(V(1, 1, 1), V(9, 9, 9))
        v2 = 4.
        idb1 = id(b1)
        cppobj = str(b1.this)
        bsum = b1 + v2
        b1 += v2
        # Inplace addition should not change the Python object identity:
        self.assertEqual(id(b1), idb1)
        # The underlying C++ object pointer should be unchanged too:
        self.assertEqual(str(b1.this), cppobj)
        for obj in b1, bsum:
            self.assertLess(IMP.algebra.get_distance(b1.get_corner(0),
                                                     V(-3, -3, -3)), 1e-4)
            self.assertLess(IMP.algebra.get_distance(b1.get_corner(1),
                                                     V(13, 13, 13)), 1e-4)

    def test_default_kd(self):
        """Test default KD constructor"""
        if IMP.base.get_check_level() >= IMP.base.USAGE:
            self.assertRaises(IMP.base.UsageException,
                              IMP.algebra.BoundingBoxKD)

    def test_bounding_box_nd(self):
        """Test BoundingBox<N> operations for unusual N"""
        for N in (-1,1,2,4,5,6):
            if N == -1:
                clsdim = 'K'
                dim = 5
            else:
                clsdim = '%d' % N
                dim = N
            V = getattr(IMP.algebra, "Vector%sD" % clsdim)
            B = getattr(IMP.algebra, "BoundingBox%sD" % clsdim)
            v1 = V([0] * dim)
            v2 = V([2] * dim)
            b = B(v1, v2)
            if N == -1:
                c = IMP.algebra.get_cube_kd(5, 1.0)
                ub = IMP.algebra.get_unit_bounding_box_kd(5)
                self.assertRaises(IMP.base.InternalException,
                                  IMP.algebra.get_vertices, b)
            else:
                c = getattr(IMP.algebra, "get_cube_%sd" % clsdim)(1.0)
                vertices = IMP.algebra.get_vertices(b)
                self.assertEqual(len(vertices), 2 ** N)
                ub = getattr(IMP.algebra,
                             "get_unit_bounding_box_%sd" % clsdim)()
            inters = IMP.algebra.get_intersection(b, b)
            uni = IMP.algebra.get_union(b, b)
            self.assertTrue(IMP.algebra.get_interiors_intersect(b, b))
            self.assertTrue(b.get_contains(v1))
            self.assertEqual(b.get_dimension(), dim)
            self.assertLess(IMP.algebra.get_distance(b.get_corner(0), v1), 1e-4)
            sio = StringIO.StringIO()
            b.show(sio)

if __name__ == '__main__':
    IMP.test.main()
