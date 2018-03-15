import utils
import os
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm

class Tests(unittest.TestCase):
    def test_system(self):
        """Test System class"""
        s = ihm.System('test system')
        self.assertEqual(s.name, 'test system')

    def test_entity(self):
        """Test Entity class"""
        e1 = ihm.Entity('ABCD', description='foo')
        # Should compare identical if sequences are the same
        e2 = ihm.Entity('ABCD', description='bar')
        e3 = ihm.Entity('ABCDE', description='foo')
        self.assertEqual(e1, e2)
        self.assertNotEqual(e1, e3)

    def test_asym_range(self):
        """Test AsymUnitRange class"""
        e = ihm.Entity('ABCDAB')
        a = ihm.AsymUnit(e)
        a._id = 42
        self.assertEqual(a.seq_id_range, (1,6))
        r = a(3,4)
        self.assertEqual(r.seq_id_range, (3,4))
        self.assertEqual(r._id, 42)
        self.assertEqual(r.entity, e)

    def test_assembly_component_entity(self):
        """Test AssemblyComponent created from an entity"""
        e = ihm.Entity('ABCD')
        c = ihm.AssemblyComponent(e)
        self.assertEqual(c.entity, e)
        self.assertEqual(c.asym, None)

    def test_assembly_component_asym(self):
        """Test AssemblyComponent created from an asym unit"""
        e = ihm.Entity('ABCD')
        a = ihm.AsymUnit(e)
        c = ihm.AssemblyComponent(a)
        self.assertEqual(c.entity, e)
        self.assertEqual(c.asym, a)

    def test_assembly_component_seqrange_entity(self):
        """Test AssemblyComponent default seq range from an entity"""
        e = ihm.Entity('ABCD')
        c = ihm.AssemblyComponent(e)
        self.assertEqual(c.seq_id_range, (1, 4))

    def test_assembly_component_seqrange_asym_unit(self):
        """Test AssemblyComponent default seq range from an asym unit"""
        e = ihm.Entity('ABCD')
        a = ihm.AsymUnit(e)
        c = ihm.AssemblyComponent(a)
        self.assertEqual(c.seq_id_range, (1, 4))

    def test_assembly_component_given_seqrange(self):
        """Test AssemblyComponent with a seq range"""
        e = ihm.Entity('ABCD')
        c = ihm.AssemblyComponent(e, (2,3))
        self.assertEqual(c.seq_id_range, (2, 3))

    def test_assembly(self):
        """Test Assembly class"""
        e1 = ihm.Entity('ABCD')
        e2 = ihm.Entity('ABC')
        c = ihm.AssemblyComponent(e1)
        a = ihm.Assembly([c, e2], name='foo', description='bar')
        self.assertEqual(a.name, 'foo')
        self.assertEqual(a.description, 'bar')

if __name__ == '__main__':
    unittest.main()
