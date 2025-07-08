import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.representation


class Tests(unittest.TestCase):

    def test_segment(self):
        """Test Segment base class"""
        seg = ihm.representation.Segment()  # does nothing
        _ = seg._get_report()

    def test_atomic_segment(self):
        """Test AtomicSegment class"""
        asym = ihm.AsymUnit(ihm.Entity('A' * 30), "testdetail")
        s = ihm.representation.AtomicSegment(
            asym_unit=asym(1, 10), rigid=True, starting_model=None)
        self.assertEqual(s.asym_unit.seq_id_range, (1, 10))
        self.assertEqual(s.primitive, 'atomistic')
        self.assertEqual(s.granularity, 'by-atom')
        self.assertIsNone(s.count)
        self.assertEqual(s.rigid, True)
        self.assertEqual(s._get_report(), "testdetail 1-10 as rigid atoms")

    def test_residue_segment(self):
        """Test ResidueSegment class"""
        asym = ihm.AsymUnit(ihm.Entity('AAAA'), "testdetail")
        s = ihm.representation.ResidueSegment(
            asym_unit=asym, rigid=True, primitive='sphere')
        self.assertEqual(s.asym_unit.seq_id_range, (1, 4))
        self.assertEqual(s.primitive, 'sphere')
        self.assertEqual(s.granularity, 'by-residue')
        self.assertIsNone(s.count)
        self.assertEqual(s.rigid, True)
        self.assertEqual(s._get_report(), "testdetail 1-4 as rigid residues")

    def test_residue_segment_starting_model(self):
        """Test ResidueSegment class with starting model"""
        class MockObject:
            pass
        asym = ihm.AsymUnit(ihm.Entity('AA'), "testdetail")
        sm = MockObject()
        s = ihm.representation.ResidueSegment(
            asym_unit=asym, rigid=False, primitive='sphere',
            starting_model=sm)
        self.assertEqual(
            s._get_report(),
            "testdetail 1-2 as flexible residues (from starting model)")
        sm._id = '42'
        self.assertEqual(
            s._get_report(),
            "testdetail 1-2 as flexible residues (from starting model 42)")

    def test_multi_residue_segment(self):
        """Test MultiResidueSegment class"""
        asym = ihm.AsymUnit(ihm.Entity('AAAA'))
        s = ihm.representation.MultiResidueSegment(
            asym_unit=asym, rigid=True, primitive='sphere')
        self.assertEqual(s.primitive, 'sphere')
        self.assertEqual(s.granularity, 'multi-residue')
        self.assertIsNone(s.count)
        self.assertEqual(s.rigid, True)

    def test_feature_segment(self):
        """Test FeatureSegment class"""
        asym = ihm.AsymUnit(ihm.Entity('AAAA'), "testdetail")
        s = ihm.representation.FeatureSegment(
            asym_unit=asym, rigid=True, primitive='sphere', count=2)
        self.assertEqual(s.primitive, 'sphere')
        self.assertEqual(s.granularity, 'by-feature')
        self.assertEqual(s.count, 2)
        self.assertEqual(s.rigid, True)
        self.assertEqual(s._get_report(),
                         'testdetail 1-4 as 2 rigid features (sphere)')

    def test_representation(self):
        """Test Representation class"""
        asym = ihm.AsymUnit(ihm.Entity('AAAA'))
        s = ihm.representation.AtomicSegment(asym_unit=asym, rigid=True)
        r = ihm.representation.Representation()
        r.append(s)
        self.assertEqual(len(r), 1)


if __name__ == '__main__':
    unittest.main()
