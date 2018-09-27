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
        self.assertEqual([c[i] for i in range(24,28)],
                         ['Y', 'Z', 'AA', 'AB'])
        self.assertEqual([c[i] for i in range(50,54)],
                         ['AY', 'AZ', 'BA', 'BB'])
        self.assertEqual([c[i] for i in range(700,704)],
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
        obj1a = DummyObj(42) # obj1a and 1b are identical
        obj1b = DummyObj(42)
        obj2 = DummyObj(34)
        obj3 = DummyObj(23) # obj3 already has an id
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

if __name__ == '__main__':
    unittest.main()
