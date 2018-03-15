import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.startmodel

class Tests(unittest.TestCase):

    def test_source(self):
        """Test Source base class"""
        class MockModel(object):
            seq_id_begin = 10
            seq_id_end = 100
        s = ihm.startmodel.Source()
        r = s.get_seq_id_range(MockModel())
        self.assertEqual(r, (10,100))

    def test_pdb_source(self):
        """Test PDBSource class"""
        class MockModel(object):
            seq_id_begin = 1
            seq_id_end = 100
        m = MockModel()
        p = ihm.startmodel.PDBSource('1abc', 'A', metadata=[])
        self.assertEqual(p.source, 'experimental model')
        self.assertEqual(p.get_seq_id_range(m), (1, 100))

    def test_template_source_pdb(self):
        """Test TemplateSource class, where template is from PDB"""
        class MockModel(object):
            seq_id_begin = 10
            seq_id_end = 100
        m = MockModel()
        for code in ('1abcA', '1abcA_2'):
            p = ihm.startmodel.TemplateSource(tm_code='1abcA',
                    tm_seq_id_begin=30, tm_seq_id_end=90, seq_id_begin=1,
                    chain_id='G', seq_id_end=90, seq_id=42.)
            self.assertEqual(p.source, 'comparative model')
            self.assertEqual(p.tm_db_code, '1ABC')
            self.assertEqual(p.tm_chain_id, 'A')
            self.assertEqual(p.get_seq_id_range(m), (10, 90))

    def test_template_source_unknown(self):
        """Test TemplateSource class, where template is not in PDB"""
        class MockModel(object):
            seq_id_begin = 10
            seq_id_end = 100
        m = MockModel()
        p = ihm.startmodel.TemplateSource(tm_code='fooA', tm_seq_id_begin=30,
                    tm_seq_id_end=90, seq_id_begin=1, chain_id='G',
                    seq_id_end=90, seq_id=42.)
        self.assertEqual(p.source, 'comparative model')
        self.assertEqual(p.tm_db_code, None)
        self.assertEqual(p.tm_chain_id, 'A')
        self.assertEqual(p.get_seq_id_range(m), (10, 90))

    def test_unknown_source(self):
        """Test UnknownSource class"""
        class MockDataset(object):
            data_type = 'Comparative model'
        class MockStartModel(object):
            seq_id_begin = 10
            seq_id_end = 100
        d = MockDataset()
        p = ihm.startmodel.UnknownSource(d, 'A')
        self.assertEqual(p.source, 'comparative model')
        self.assertEqual(p.get_seq_id_range(MockStartModel()), (10, 100))

    def test_pdb_helix(self):
        """Test PDBHelix class"""
        p = ihm.startmodel.PDBHelix(
              "HELIX   10  10 ASP A  607  GLU A  624  1"
              "                                  18   ")
        self.assertEqual(p.helix_id, '10')
        self.assertEqual(p.start_asym, 'A')
        self.assertEqual(p.start_resnum, 607)
        self.assertEqual(p.end_asym, 'A')
        self.assertEqual(p.end_resnum, 624)
        self.assertEqual(p.helix_class, 1)
        self.assertEqual(p.length, 18)

    def test_starting_model(self):
        """Test StartingModel class"""
        e1 = ihm.Entity('AAAA')
        asym = ihm.AsymUnit(e1)
        s1 = ihm.startmodel.PDBSource('1abc', 'C', [])
        s2 = ihm.startmodel.PDBSource('2xyz', 'D', [])
        s = ihm.startmodel.StartingModel(asym, 'mock_dataset', 'A',
                                         [s1, s2], offset=10)
        self.assertEqual(s.get_seq_id_range_all_sources(), (1,4))


if __name__ == '__main__':
    unittest.main()
