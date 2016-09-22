from __future__ import print_function
import IMP.test
import IMP.pmi.metadata
import IMP.pmi.representation

class Tests(IMP.test.TestCase):

    def test_software(self):
        """Test metadata.Software"""
        s = IMP.pmi.metadata.Software(name='test', classification='test code',
                                      description='Some test program',
                                      url='http://salilab.org')
        self.assertEqual(s.name, 'test')

    def test_citation(self):
        """Test metadata.Citation"""
        s = IMP.pmi.metadata.Citation(title='Test paper', journal='J Mol Biol',
                                      volume=45, page_range=(1,20), year=2016,
                                      authors=['Smith, A.', 'Jones, B.'],
                                      doi='10.2345/S1384107697000225',
                                      pmid='1234')
        self.assertEqual(s.title, 'Test paper')

    def test_repository(self):
        """Test metadata.Repository"""
        s = IMP.pmi.metadata.Repository(doi='10.5281/zenodo.46266', root='..')
        self.assertEqual(s._root, '..')
        f = s.get_path('../foo')
        self.assertEqual(f.path, 'foo')

    def test_repr_add(self):
        """Test Representation.add_metadata()"""
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.add_metadata(IMP.pmi.metadata.Repository(
                                   doi='10.5281/zenodo.46266', root='..'))
        self.assertEqual(r._metadata[0]._root, '..')

    def test_database_location(self):
        """Test DatabaseLocation class"""
        dl1 = IMP.pmi.metadata.DatabaseLocation('mydb', 'abc', version=1)
        dl2 = IMP.pmi.metadata.DatabaseLocation('mydb', 'abc', version=1)
        self.assertEqual(dl1, dl2)
        dl3 = IMP.pmi.metadata.DatabaseLocation('mydb', 'abc', version=2)
        self.assertNotEqual(dl1, dl3)
        # details can change without affecting equality
        dl4 = IMP.pmi.metadata.DatabaseLocation('mydb', 'abc', version=1,
                                                details='foo')
        self.assertEqual(dl1, dl4)
        self.assertEqual(dl1.db_name, 'mydb')
        self.assertEqual(dl1.access_code, 'abc')
        self.assertEqual(dl1.version, 1)
        self.assertEqual(dl1.details, None)

    def test_emdb_location(self):
        """Test EMDBLocation class"""
        d = IMP.pmi.metadata.EMDBLocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'EMDB')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_pdb_location(self):
        """Test PDBLocation class"""
        d = IMP.pmi.metadata.PDBLocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'PDB')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_massive_location(self):
        """Test MassIVELocation class"""
        d = IMP.pmi.metadata.MassIVELocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'MassIVE')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_repo_file_location(self):
        """Test RepositoryFileLocation class"""
        d = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'mypath')
        self.assertEqual(d.doi, 'mydoi')
        self.assertEqual(d.path, 'mypath')
        d2 = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'mypath')
        self.assertEqual(d, d2)
        d3 = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'otherpath')
        self.assertNotEqual(d, d3)
        d4 = IMP.pmi.metadata.RepositoryFileLocation('otherdoi', 'mypath')
        self.assertNotEqual(d, d4)

    def test_default_file_location(self):
        """Test get_default_file_location function"""
        d = IMP.pmi.metadata.get_default_file_location('foo')
        self.assertEqual(d.doi, None)
        self.assertEqual(d.path, 'foo')

    def test_dataset_add_primary(self):
        """Test Dataset.add_primary()"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d1 = IMP.pmi.metadata.CXMSDataset(loc)
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'b')
        d2 = IMP.pmi.metadata.MassSpecDataset(loc)
        d1.add_primary(d2)
        self.assertEqual(d1._primaries, {d2:None})
        # Ignore duplicates
        d1.add_primary(d2)
        self.assertEqual(d1._primaries, {d2:None})

    def test_cxms_dataset(self):
        """Test CXMSDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.CXMSDataset(loc)
        self.assertEqual(d._data_type, 'CX-MS data')

    def test_mass_spec_dataset(self):
        """Test MassSpecDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.MassSpecDataset(loc)
        self.assertEqual(d._data_type, 'Mass Spectrometry data')

    def test_em_density_dataset(self):
        """Test EMDensityDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.EMDensityDataset(loc)
        self.assertEqual(d._data_type, '3DEM volume')

    def test_pdb_dataset(self):
        """Test PDBDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.PDBDataset(loc)
        self.assertEqual(d._data_type, 'Experimental model')

    def test_comp_model_dataset(self):
        """Test ComparativeModelDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.ComparativeModelDataset(loc)
        self.assertEqual(d._data_type, 'Comparative model')

    def test_em_micrographs_dataset(self):
        """Test EMMicrographsDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.EMMicrographsDataset(loc, 400)
        self.assertEqual(d._data_type, 'EM raw micrographs')
        self.assertEqual(d.number, 400)
        d2 = IMP.pmi.metadata.EMMicrographsDataset(loc, 400)
        self.assertEqual(d, d2)
        # Not equal if number differs
        d3 = IMP.pmi.metadata.EMMicrographsDataset(loc, 600)
        self.assertNotEqual(d, d3)

    def test_em2d_class_dataset(self):
        """Test EM2DClassDataset"""
        loc = IMP.pmi.metadata.RepositoryFileLocation('mydoi', 'a')
        d = IMP.pmi.metadata.EM2DClassDataset(loc)
        self.assertEqual(d._data_type, '2DEM class average')

if __name__ == '__main__':
    IMP.test.main()
