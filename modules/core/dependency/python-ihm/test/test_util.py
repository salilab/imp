import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.util


class Tests(unittest.TestCase):
    def test_asym_ids(self):
        """Test _AsymIDs class"""
        c = ihm.util._AsymIDs()
        self.assertEqual([c[i] for i in range(0, 4)],
                         ['A', 'B', 'C', 'D'])
        self.assertEqual([c[i] for i in range(24, 28)],
                         ['Y', 'Z', 'AA', 'AB'])
        self.assertEqual([c[i] for i in range(50, 54)],
                         ['AY', 'AZ', 'BA', 'BB'])
        self.assertEqual([c[i] for i in range(700, 704)],
                         ['ZY', 'ZZ', 'AAA', 'AAB'])

    def test_remove_id(self):
        """Test _remove_id utility function"""
        class DummyObj(object):
            pass
        o = DummyObj()
        ihm.util._remove_id(o)
        o._id = 42
        ihm.util._remove_id(o)
        self.assertFalse(hasattr(o, '_id'))

    def test_assign_id(self):
        """Test _assign_id utility function"""
        class DummyObj(object):
            def __init__(self, hashval):
                self.hashval = hashval

            def __eq__(self, other):
                return self.hashval == other.hashval

            def __hash__(self):
                return self.hashval
        seen_objs = {}
        obj_by_id = []
        obj1a = DummyObj(42)  # obj1a and 1b are identical
        obj1b = DummyObj(42)
        obj2 = DummyObj(34)
        obj3 = DummyObj(23)  # obj3 already has an id
        obj3._id = 'foo'
        for obj in (obj1a, obj1b, obj2, obj3):
            ihm.util._assign_id(obj, seen_objs, obj_by_id)
        self.assertEqual(obj1a._id, 1)
        self.assertEqual(obj1b._id, 1)
        self.assertEqual(obj2._id, 2)
        self.assertEqual(obj3._id, 'foo')
        self.assertEqual(obj_by_id, [obj1a, obj2])

    def test_get_relative_path(self):
        """Test get_relative_path()"""
        if os.path.sep == '/':
            self.assertEqual(ihm.util._get_relative_path('/foo/bar',
                                                         '/foo/bar'),
                             '/foo/bar')
            self.assertEqual(ihm.util._get_relative_path('/foo/bar', 'baz'),
                             '/foo/baz')

    def test_invert_ranges(self):
        """Test _invert_ranges function"""
        inrng = [(2, 3)]
        self.assertEqual(list(ihm.util._invert_ranges(inrng, 4)),
                         [(1, 1), (4, 4)])
        inrng = [(1, 1), (4, 7)]
        self.assertEqual(list(ihm.util._invert_ranges(inrng, 8)),
                         [(2, 3), (8, 8)])
        inrng = [(2, 2), (4, 7)]
        self.assertEqual(list(ihm.util._invert_ranges(inrng, 7)),
                         [(1, 1), (3, 3)])
        # Test with non-default start value
        inrng = [(5, 8)]
        self.assertEqual(list(ihm.util._invert_ranges(inrng, 10, start=3)),
                         [(3, 4), (9, 10)])
        self.assertEqual(list(ihm.util._invert_ranges(inrng, 20, start=10)),
                         [(9, 20)])
        self.assertEqual(list(ihm.util._invert_ranges(inrng, 4, start=1)),
                         [(1, 4)])

    def test_pred_ranges(self):
        """Test _pred_ranges function"""
        inrng = [(2, 3)]
        self.assertEqual(list(ihm.util._pred_ranges(inrng, 4)),
                         [(1, 1, False), (2, 3, True), (4, 4, False)])
        inrng = [(1, 1), (4, 7)]
        self.assertEqual(list(ihm.util._pred_ranges(inrng, 8)),
                         [(1, 1, True), (2, 3, False), (4, 7, True),
                          (8, 8, False)])
        inrng = [(2, 2), (4, 7)]
        self.assertEqual(list(ihm.util._pred_ranges(inrng, 7)),
                         [(1, 1, False), (2, 2, True), (3, 3, False),
                          (4, 7, True)])

    def test_combine_ranges(self):
        """Test _combine_ranges function"""
        inrng = [(8, 10), (1, 2), (3, 4)]
        self.assertEqual(list(ihm.util._combine_ranges(inrng)),
                         [(1, 4), (8, 10)])
        inrng = [(1, 10), (3, 4)]
        self.assertEqual(list(ihm.util._combine_ranges(inrng)), [(1, 10)])
        inrng = [(1, 2), (1, 4)]
        self.assertEqual(list(ihm.util._combine_ranges(inrng)), [(1, 4)])
        inrng = [(1, 2), (4, 4)]
        self.assertEqual(list(ihm.util._combine_ranges(inrng)),
                         [(1, 2), (4, 4)])
        self.assertEqual(list(ihm.util._combine_ranges([])), [])

    def test_make_range_from_list(self):
        """Test _make_range_from_list function"""
        rr = []
        self.assertEqual(list(ihm.util._make_range_from_list(rr)), [])
        rr = [1, 2, 5, 6, 10, 11, 20]
        self.assertEqual(list(ihm.util._make_range_from_list(rr)),
                         [[1, 2], [5, 6], [10, 11], [20, 20]])

    def test_get_codes(self):
        """Test _get_codes function"""
        self.assertEqual(tuple(ihm.util._get_codes(None)), ())
        self.assertEqual(tuple(ihm.util._get_codes(ihm.unknown)), ())
        self.assertEqual(tuple(ihm.util._get_codes("TWC\nAA(FOO)T")),
                         ('T', 'W', 'C', 'A', 'A', 'FOO', 'T'))


if __name__ == '__main__':
    unittest.main()
