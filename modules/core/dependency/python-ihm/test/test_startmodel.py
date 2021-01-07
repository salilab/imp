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
        p = ihm.startmodel.Template(
            dataset=dataset, asym_id='G', seq_id_range=(1, 90),
            template_seq_id_range=(30, 90), sequence_identity=42.)
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
        self.assertEqual(s.get_atoms(), [])
        self.assertEqual(s.get_seq_dif(), [])

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

    def test_add_atom(self):
        """Test StartingModel.add_atom()"""
        atoms = ['atom1', 'atom2']
        e1 = ihm.Entity('AAAA')
        asym = ihm.AsymUnit(e1)
        s = ihm.startmodel.StartingModel(asym, 'mock_dataset', 'A', offset=10)
        s.add_atom(atoms[0])
        s.add_atom(atoms[1])
        self.assertEqual(s._atoms, atoms)

    def test_add_seq_dif(self):
        """Test StartingModel.add_seq_dif()"""
        seq_difs = ['sd1', 'sd2']
        e1 = ihm.Entity('AAAA')
        asym = ihm.AsymUnit(e1)
        s = ihm.startmodel.StartingModel(asym, 'mock_dataset', 'A', offset=10)
        s.add_seq_dif(seq_difs[0])
        s.add_seq_dif(seq_difs[1])
        self.assertEqual(s._seq_difs, seq_difs)


if __name__ == '__main__':
    unittest.main()
