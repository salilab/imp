import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.startmodel

class Tests(unittest.TestCase):

    def test_template(self):
        """Test Template class"""
        class MockObject(object):
            pass
        dataset = MockObject()
        p = ihm.startmodel.Template(dataset=dataset, asym_id='G',
                    seq_id_range=(1,90), template_seq_id_range=(30,90),
                    sequence_identity=42.)
        self.assertEqual(p.asym_id, 'G')

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
        """Test StartingModel class, no templates"""
        e1 = ihm.Entity('AAAA')
        asym = ihm.AsymUnit(e1)
        s = ihm.startmodel.StartingModel(asym, 'mock_dataset', 'A', offset=10)
        self.assertEqual(s.get_seq_id_range_all_templates(), (1,4))
        self.assertEqual(s.get_atoms(), [])
        self.assertEqual(s.get_seq_dif(), [])

    def test_starting_model_templates(self):
        """Test StartingModel class, with templates"""
        e1 = ihm.Entity('A' * 30)
        asym = ihm.AsymUnit(e1)
        # This template extends outside the range of the starting model;
        # should be truncated to start at 5
        s1 = ihm.startmodel.Template(dataset='1abc', asym_id='C',
                             seq_id_range=(0,9), # 1,10 in IHM numbering
                             template_seq_id_range=(101,110),
                             sequence_identity=30.)
        # This should extend the range to 20
        s2 = ihm.startmodel.Template(dataset='2xyz', asym_id='D',
                             seq_id_range=(14, 19), # 15,20 in IHM numbering
                             template_seq_id_range=(115,120),
                             sequence_identity=40.)
        s = ihm.startmodel.StartingModel(asym(5,25), 'mock_dataset', 'A',
                                         [s1, s2], offset=1)
        self.assertEqual(s.get_seq_id_range_all_templates(), (5,20))

    def test_seq_dif(self):
        """Test SeqDif class"""
        sd = ihm.startmodel.SeqDif(db_seq_id=10, seq_id=20, db_comp_id='PHE')
        self.assertEqual(sd.db_seq_id, 10)

    def test_mse_seq_dif(self):
        """Test MSESeqDif class"""
        sd = ihm.startmodel.MSESeqDif(db_seq_id=10, seq_id=20)
        self.assertEqual(sd.db_comp_id, 'MSE')
        self.assertEqual(sd.details,
                         'Conversion of modified residue MSE to MET')


if __name__ == '__main__':
    unittest.main()
