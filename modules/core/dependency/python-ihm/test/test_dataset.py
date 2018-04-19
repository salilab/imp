import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.dataset
import ihm.location

def _make_test_file(fname):
    with open(fname, 'w') as fh:
        fh.write('contents')

class Tests(unittest.TestCase):

    def test_dataset(self):
        """Test Dataset base class"""
        l = ihm.location.PDBLocation('1abc', version='foo', details='bar')
        d = ihm.dataset.Dataset(l)
        self.assertEqual(len(d.parents), 0)

        l2 = ihm.location.PDBLocation('1xyz', version='foo', details='bar')
        d2 = ihm.dataset.Dataset(l2)
        d.parents.append(d2)
        self.assertEqual(len(d.parents), 1)
        self.assertNotEqual(d, d2)

    def test_add_primary_no_parents(self):
        """Test add_primary() method, no parents"""
        l1 = ihm.location.PDBLocation('1abc', version='foo', details='bar')
        d1 = ihm.dataset.Dataset(l1)

        l2 = ihm.location.PDBLocation('1xyz', version='foo', details='bar')
        d2 = ihm.dataset.Dataset(l2)

        d1.add_primary(d2)
        self.assertEqual(d1.parents, [d2])

    def test_add_primary_one_parent(self):
        """Test add_primary() method, one parent"""
        l1 = ihm.location.PDBLocation('1abc', version='foo', details='bar')
        d1 = ihm.dataset.Dataset(l1)

        l2 = ihm.location.PDBLocation('1xyz', version='foo', details='bar')
        d2 = ihm.dataset.Dataset(l2)

        l3 = ihm.location.PDBLocation('2def', version='foo', details='bar')
        d3 = ihm.dataset.Dataset(l3)

        d1.parents.append(d2)

        d1.add_primary(d3)
        self.assertEqual(d1.parents, [d2])
        self.assertEqual(d2.parents, [d3])

    def test_add_primary_two_parents(self):
        """Test add_primary() method, two parents"""
        l1 = ihm.location.PDBLocation('1abc', version='foo', details='bar')
        d1 = ihm.dataset.Dataset(l1)

        l2 = ihm.location.PDBLocation('1xyz', version='foo', details='bar')
        d2 = ihm.dataset.Dataset(l2)

        l3 = ihm.location.PDBLocation('2def', version='foo', details='bar')
        d3 = ihm.dataset.Dataset(l3)

        l4 = ihm.location.PDBLocation('2ghi', version='foo', details='bar')
        d4 = ihm.dataset.Dataset(l3)

        d1.parents.extend((d2, d3))

        self.assertRaises(ValueError, d1.add_primary, d4)

    def test_cxms_dataset(self):
        """Test CXMSDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.CXMSDataset(loc)
        self.assertEqual(d.data_type, 'CX-MS data')

    def test_mass_spec_dataset(self):
        """Test MassSpecDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.MassSpecDataset(loc)
        self.assertEqual(d.data_type, 'Mass Spectrometry data')

    def test_em_density_dataset(self):
        """Test EMDensityDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.EMDensityDataset(loc)
        self.assertEqual(d.data_type, '3DEM volume')

    def test_pdb_dataset(self):
        """Test PDBDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.PDBDataset(loc)
        self.assertEqual(d.data_type, 'Experimental model')

    def test_comp_model_dataset(self):
        """Test ComparativeModelDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.ComparativeModelDataset(loc)
        self.assertEqual(d.data_type, 'Comparative model')

    def test_int_model_dataset(self):
        """Test IntegrativeModelDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.IntegrativeModelDataset(loc)
        self.assertEqual(d.data_type, 'Integrative model')

    def test_em2d_class_dataset(self):
        """Test EM2DClassDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.EM2DClassDataset(loc)
        self.assertEqual(d.data_type, '2DEM class average')

    def test_em_micrographs_dataset(self):
        """Test EMMicrographsDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.EMMicrographsDataset(loc)
        self.assertEqual(d.data_type, 'EM raw micrographs')

    def test_sas_dataset(self):
        """Test SASDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.SASDataset(loc)
        self.assertEqual(d.data_type, 'SAS data')

    def test_fret_dataset(self):
        """Test FRETDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.FRETDataset(loc)
        self.assertEqual(d.data_type, 'Single molecule FRET data')

    def test_y2h_dataset(self):
        """Test YeastTwoHybridDataset"""
        loc = ihm.location.FileLocation(repo='mydoi', path='a')
        d = ihm.dataset.YeastTwoHybridDataset(loc)
        self.assertEqual(d.data_type, 'Yeast two-hybrid screening data')

    def test_duplicate_datasets_details(self):
        """Datasets with differing details should be considered duplicates"""
        with utils.temporary_directory() as tmpdir:
            fname = os.path.join(tmpdir, 'test.pdb')
            _make_test_file(fname)
            l1 = ihm.location.InputFileLocation(fname, details='test details')
            d1 = ihm.dataset.PDBDataset(l1)

            l2 = ihm.location.InputFileLocation(fname, details='other details')
            d2 = ihm.dataset.PDBDataset(l2)
            self.assertEqual(l1, l2)

    def test_duplicate_locations(self):
        """Datasets with same location should be considered duplicates"""
        with utils.temporary_directory() as tmpdir:
            fname1 = os.path.join(tmpdir, 'test1.pdb')
            fname2 = os.path.join(tmpdir, 'test2.pdb')
            _make_test_file(fname1)
            _make_test_file(fname2)
            loc1 = ihm.location.InputFileLocation(fname1)
            loc2 = ihm.location.InputFileLocation(fname2)

            # Identical datasets in the same location aren't duplicated
            pdb1 = ihm.dataset.PDBDataset(loc1)
            pdb2 = ihm.dataset.PDBDataset(loc1)
            self.assertEqual(pdb1, pdb2)

            # Datasets in different locations are OK
            pdb1 = ihm.dataset.PDBDataset(loc1)
            pdb2 = ihm.dataset.PDBDataset(loc2)
            self.assertNotEqual(pdb1, pdb2)


if __name__ == '__main__':
    unittest.main()
