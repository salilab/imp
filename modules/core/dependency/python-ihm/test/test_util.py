import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.util


class HashObj:
    def __init__(self, hashval, id=None):
        self.hashval = hashval
        if id is not None:
            self._id = id

    def __eq__(self, other):
        return self.hashval == other.hashval

    def __hash__(self):
        return self.hashval


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
        class DummyObj:
            pass
        o = DummyObj()
        ihm.util._remove_id(o)
        o._id = 42
        ihm.util._remove_id(o)
        self.assertFalse(hasattr(o, '_id'))

    def test_assign_id(self):
        """Test _assign_id utility function"""
        seen_objs = {}
        obj_by_id = []
        obj1a = HashObj(42)  # obj1a and 1b are identical
        obj1b = HashObj(42)
        obj2 = HashObj(34)
        obj3 = HashObj(23)  # obj3 already has an id
        obj3._id = 'foo'
        for obj in (obj1a, obj1b, obj2, obj3):
            ihm.util._assign_id(obj, seen_objs, obj_by_id)
        self.assertEqual(obj1a._id, 1)
        self.assertEqual(obj1b._id, 1)
        self.assertEqual(obj2._id, 2)
        self.assertEqual(obj3._id, 'foo')
        self.assertEqual(obj_by_id, [obj1a, obj2])

    def test_hash_assign_ids_class(self):
        """Test _HashAssignIDs class"""
        # Two identical objects: should get compressed to one ID
        objs = [HashObj(42), HashObj(42)]
        c = ihm.util._HashAssignIDs(objs)
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1)])
        self.assertEqual([obj._id for obj in objs], [1, 1])

        # Two different objects: should get sequential IDs
        c = ihm.util._HashAssignIDs([HashObj(42), HashObj(24)])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1), (24, 2)])

        # Existing "numeric" string ID should be retained; other objects
        # should get IDs larger than it
        c = ihm.util._HashAssignIDs([HashObj(34), HashObj(24, id='42')])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(24, '42'), (34, 43)])

        # Existing int ID should be retained; other objects
        # should get IDs larger than it
        c = ihm.util._HashAssignIDs([HashObj(34), HashObj(24, id=42)])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(24, 42), (34, 43)])

        # Existing IDs should come out sorted numerically if possible
        c = ihm.util._HashAssignIDs([HashObj(34, id=99), HashObj(24, id=42),
                                     HashObj(33, id=2), HashObj(36, id='foo'),
                                     HashObj(37, id='bar'),
                                     HashObj(38, id='44'),
                                     HashObj(39, id='24')])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(36, 'foo'), (37, 'bar'), (33, 2), (39, '24'),
                          (24, 42), (38, '44'), (34, 99)])

        # Non-numeric existing ID should be retained but not affect numbering
        # of other objects
        c = ihm.util._HashAssignIDs([HashObj(34), HashObj(24, id='foo')])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(24, 'foo'), (34, 1)])

        # Identical objects with different IDs: should be compressed to one
        objs = [HashObj(24, id='foo'), HashObj(24, id='bar')]
        c = ihm.util._HashAssignIDs(objs)
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(24, 'foo')])
        self.assertEqual([obj._id for obj in objs], ['foo', 'foo'])

        # Different objects with the same ID: all but the first should be
        # renumbered
        c = ihm.util._HashAssignIDs([HashObj(34, id='foo'),
                                     HashObj(24, id='foo')])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(34, 'foo'), (24, 1)])

        # Should also work with a callable
        def get_objs():
            yield HashObj(42)
            yield HashObj(24)

        c = ihm.util._HashAssignIDs(get_objs)
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1), (24, 2)])

    def test_hash_assign_ids_class_hash_func(self):
        """Test _HashAssignIDs class with custom hash_func"""
        def hash_func(x):
            # hash==42 and hash==142 should now compare equal
            return x.hashval % 100

        # Two identical objects: should get compressed to one ID
        objs = [HashObj(42), HashObj(142)]
        c = ihm.util._HashAssignIDs(objs, hash_func=hash_func)
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1)])
        self.assertEqual([obj._id for obj in objs], [1, 1])

        # Two different objects: should get sequential IDs
        c = ihm.util._HashAssignIDs([HashObj(42), HashObj(24)],
                                    hash_func=hash_func)
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1), (24, 2)])

    def test_enumerate_assign_ids_class(self):
        """Test _EnumerateAssignIDs class"""
        # Two identical objects: should get sequential IDs
        objs = [HashObj(42), HashObj(42)]
        c = ihm.util._EnumerateAssignIDs(objs)
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1), (42, 2)])

        # Two different objects: should get sequential IDs
        c = ihm.util._EnumerateAssignIDs([HashObj(42), HashObj(24)])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(42, 1), (24, 2)])

        # Existing int ID should be retained; other objects
        # should get IDs larger than it
        c = ihm.util._EnumerateAssignIDs([HashObj(34), HashObj(24, id=42)])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(24, 42), (34, 43)])

        # Different objects with the same ID: all but the first should be
        # renumbered
        c = ihm.util._EnumerateAssignIDs([HashObj(34, id='foo'),
                                         HashObj(24, id='foo')])
        self.assertEqual([(obj.hashval, obj._id) for obj in c.assign_all()],
                         [(34, 'foo'), (24, 1)])

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

    def test_intlike(self):
        """Test _intlike function"""
        self.assertEqual(ihm.util._intlike(42), 42)
        self.assertEqual(ihm.util._intlike(42.34), 42)
        self.assertEqual(ihm.util._intlike("42"), 42)
        self.assertEqual(ihm.util._intlike("42.34"), 0)
        self.assertEqual(ihm.util._intlike("foo"), 0)


if __name__ == '__main__':
    unittest.main()
