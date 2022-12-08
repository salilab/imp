from __future__ import print_function
import IMP.test
import IMP.algebra
import io
import os
import math

displayit = False
if displayit:
    import IMP.display

from IMP.algebra import *
import pickle


class Tests(IMP.test.TestCase):

    def test_magnitude(self):
        """Check dense log grid of ints"""
        print("construct")
        bb = BoundingBox3D(Vector3D(1, 1, 1), Vector3D(15, 15, 15))
        sz = [5, 5, 5]
        le = LogEmbedding3D(bb, Vector3D(2.0, 2.0, 2.0), sz)
        g = DenseIntLogGrid3D(sz, le)
        bbo = g.get_bounding_box()
        print(bb, bbo)
        if displayit:
            w = IMP.display.PymolWriter(self.get_tmp_file_name("log.pym"))
            bbg = IMP.display.BoundingBoxGeometry(bb)
            bbg.set_color(IMP.display.get_display_color(0))
            bbg.set_name("in")
            w.add_geometry(bbg)
            bbog = IMP.display.BoundingBoxGeometry(bbo)
            bbog.set_color(IMP.display.get_display_color(1))
            bbog.set_name("out")
            w.add_geometry(bbog)
            for i in range(0, sz[0]):
                for j in range(0, sz[0]):
                    for k in range(0, sz[0]):
                        ei = ExtendedGridIndex3D(i, j, k)
                        gi = g.get_index(ei)
                        bbi = g.get_bounding_box(ei)
                        bbog = IMP.display.BoundingBoxGeometry(bbi)
                        bbog.set_name(str(ei))
                        w.add_geometry(bbog)
                        cg = IMP.display.PointGeometry(g.get_center(ei))
                        cg.set_name("center")
                        w.add_geometry(cg)
        self.assertAlmostEqual(bbo.get_corner(1)[0], 15, delta=.1)

    def test_embedding(self):
        """Test mixed log embedding"""
        eb = IMP.algebra.LogEmbedding3D(IMP.algebra.Vector3D(0, 0, 0),
                                        IMP.algebra.Vector3D(1, 1, 1),
                                        IMP.algebra.Vector3D(1, 2, 1))
        for i in range(0, 10):
            gi = IMP.algebra.ExtendedGridIndex3D([i, i, i])
            center = eb.get_center(gi)
            print(center)

    def test_default_embedding_pickle(self):
        """Test (un-)pickle of DefaultEmbedding3D"""
        e1 = IMP.algebra.DefaultEmbedding3D(IMP.algebra.Vector3D(1,2,3),
                                            IMP.algebra.Vector3D(2,4,5))
        e2 = IMP.algebra.DefaultEmbedding3D(IMP.algebra.Vector3D(4,5,6),
                                            IMP.algebra.Vector3D(7,8,9))
        e2.foo = 'bar'
        dump = pickle.dumps((e1, e2))
        newe1, newe2 = pickle.loads(dump)

        self.assertLess(IMP.algebra.get_distance(
            e1.get_origin(), newe1.get_origin()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            e1.get_unit_cell(), newe1.get_unit_cell()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            e2.get_origin(), newe2.get_origin()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            e2.get_unit_cell(), newe2.get_unit_cell()), 1e-4)
        self.assertEqual(newe2.foo, 'bar')

        self.assertRaises(TypeError, e1._set_from_binary, 42)

    def test_log_embedding_pickle(self):
        """Test (un-)pickle of LogEmbedding3D"""
        e1 = IMP.algebra.LogEmbedding3D(IMP.algebra.Vector3D(1,2,3),
                                        IMP.algebra.Vector3D(2,4,5),
                                        IMP.algebra.Vector3D(7,8,9))
        e2 = IMP.algebra.LogEmbedding3D(IMP.algebra.Vector3D(4,5,6),
                                        IMP.algebra.Vector3D(7,8,9),
                                        IMP.algebra.Vector3D(17,18,19))
        e2.foo = 'bar'
        dump = pickle.dumps((e1, e2))
        newe1, newe2 = pickle.loads(dump)

        self.assertLess(IMP.algebra.get_distance(
            e1.get_origin(), newe1.get_origin()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            e1.get_unit_cell(), newe1.get_unit_cell()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            e2.get_origin(), newe2.get_origin()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            e2.get_unit_cell(), newe2.get_unit_cell()), 1e-4)
        self.assertEqual(newe2.foo, 'bar')

        self.assertRaises(TypeError, e1._set_from_binary, 42)

    def test_grid_index_pickle(self):
        """Test (un-)pickle of GridIndex3D"""
        g1 = IMP.algebra.GridIndex3D(1,2,3)
        g2 = IMP.algebra.GridIndex3D(4,5,6)
        g2.foo = 'bar'
        dump = pickle.dumps((g1, g2))
        newg1, newg2 = pickle.loads(dump)

        self.assertEqual(g1[0], newg1[0])
        self.assertEqual(g1[1], newg1[1])
        self.assertEqual(g1[2], newg1[2])
        self.assertEqual(g2[0], newg2[0])
        self.assertEqual(g2[1], newg2[1])
        self.assertEqual(g2[2], newg2[2])
        self.assertEqual(newg2.foo, 'bar')

        self.assertRaises(TypeError, g1._set_from_binary, 42)

    def test_extended_grid_index_pickle(self):
        """Test (un-)pickle of ExtendedGridIndex3D"""
        g1 = IMP.algebra.ExtendedGridIndex3D(1,2,3)
        g2 = IMP.algebra.ExtendedGridIndex3D(4,5,6)
        g2.foo = 'bar'
        dump = pickle.dumps((g1, g2))
        newg1, newg2 = pickle.loads(dump)

        self.assertEqual(g1[0], newg1[0])
        self.assertEqual(g1[1], newg1[1])
        self.assertEqual(g1[2], newg1[2])
        self.assertEqual(g2[0], newg2[0])
        self.assertEqual(g2[1], newg2[1])
        self.assertEqual(g2[2], newg2[2])
        self.assertEqual(newg2.foo, 'bar')

        self.assertRaises(TypeError, g1._set_from_binary, 42)

    def test_unbounded_grid_range_pickle(self):
        """Test (un-)pickle of UnboundedGridRange3D"""
        g1 = IMP.algebra.UnboundedGridRange3D()
        g2 = IMP.algebra.UnboundedGridRange3D()
        g2.foo = 'bar'
        dump = pickle.dumps((g1, g2))
        newg1, newg2 = pickle.loads(dump)

        self.assertEqual(newg2.foo, 'bar')

        self.assertRaises(TypeError, g1._set_from_binary, 42)

    def test_bounded_grid_range_pickle(self):
        """Test (un-)pickle of BoundedGridRange3D"""
        g1 = IMP.algebra.BoundedGridRange3D([1,2,3])
        g2 = IMP.algebra.BoundedGridRange3D([4,5,6])
        g2.foo = 'bar'
        dump = pickle.dumps((g1, g2))
        newg1, newg2 = pickle.loads(dump)

        self.assertEqual(g1.get_end_index(), newg1.get_end_index())
        self.assertEqual(g2.get_end_index(), newg2.get_end_index())
        self.assertEqual(newg2.foo, 'bar')

        self.assertRaises(TypeError, g1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
