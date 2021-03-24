import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.location


def _make_test_file(fname):
    with open(fname, 'w') as fh:
        fh.write('contents')


class Tests(unittest.TestCase):

    def test_database_location(self):
        """Test DatabaseLocation"""
        dl1 = ihm.location.DatabaseLocation('mydb', 'abc', version=1)
        dl2 = ihm.location.DatabaseLocation('mydb', 'abc', version=1)
        self.assertEqual(dl1, dl2)
        dl3 = ihm.location.DatabaseLocation('mydb', 'abc', version=2)
        self.assertNotEqual(dl1, dl3)
        # details can change without affecting equality
        dl4 = ihm.location.DatabaseLocation('mydb', 'abc', version=1,
                                            details='foo')
        self.assertEqual(dl1, dl4)
        self.assertEqual(dl1.db_name, 'mydb')
        self.assertEqual(dl1.access_code, 'abc')
        self.assertEqual(dl1.version, 1)
        self.assertIsNone(dl1.details)

    def test_pdb_location(self):
        """Test PDBLocation"""
        loc = ihm.location.PDBLocation('1abc', version='foo', details='bar')
        self.assertEqual(loc.db_name, 'PDB')
        self.assertEqual(loc.access_code, '1abc')
        self.assertEqual(loc.version, 'foo')
        self.assertEqual(loc.details, 'bar')

    def test_pdb_dev_location(self):
        """Test PDBDevLocation"""
        loc = ihm.location.PDBDevLocation(
            'PDBDEV_00000029', version='foo', details='bar')
        self.assertEqual(loc.db_name, 'PDB-Dev')
        self.assertEqual(loc.access_code, 'PDBDEV_00000029')
        self.assertEqual(loc.version, 'foo')
        self.assertEqual(loc.details, 'bar')

    def test_bmrb_location(self):
        """Test BMRBLocation"""
        loc = ihm.location.BMRBLocation('27600', version='foo', details='bar')
        self.assertEqual(loc.db_name, 'BMRB')
        self.assertEqual(loc.access_code, '27600')
        self.assertEqual(loc.version, 'foo')
        self.assertEqual(loc.details, 'bar')

    def test_emdb_location(self):
        """Test EMDBLocation"""
        loc = ihm.location.EMDBLocation('EMDB-123', version='foo',
                                        details='bar')
        self.assertEqual(loc.db_name, 'EMDB')
        self.assertEqual(loc.access_code, 'EMDB-123')
        self.assertEqual(loc.version, 'foo')
        self.assertEqual(loc.details, 'bar')

    def test_massive_location(self):
        """Test MassIVELocation class"""
        d = ihm.location.MassIVELocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'MASSIVE')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_proxl_location(self):
        """Test ProXLLocation class"""
        d = ihm.location.ProXLLocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'ProXL')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_empiar_location(self):
        """Test EMPIARLocation class"""
        d = ihm.location.EMPIARLocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'EMPIAR')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_sasbdb_location(self):
        """Test SASBDBLocation class"""
        d = ihm.location.SASBDBLocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'SASBDB')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_pride_location(self):
        """Test PRIDELocation class"""
        d = ihm.location.PRIDELocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'PRIDE')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_biogrid_location(self):
        """Test BioGRIDLocation class"""
        d = ihm.location.BioGRIDLocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'BioGRID')
        self.assertEqual(d.access_code, 'abc')
        self.assertEqual(d.version, 1)
        self.assertEqual(d.details, 'foo')

    def test_location(self):
        """Test Location base class"""
        loc = ihm.location.Location(details='foo')
        loc._allow_duplicates = True
        self.assertEqual(loc._eq_vals(), id(loc))
        # Locations should never compare equal to None
        self.assertNotEqual(loc, None)

    def test_file_location_local(self):
        """Test InputFileLocation with a local file"""
        # Make tmpdir under current directory, as it's not always possible to
        # get a relative path from cwd to /tmp (e.g. on Windows where they may
        # be on different drives)
        with utils.temporary_directory('.') as tmpdir:
            fname = os.path.join(tmpdir, 'test.pdb')
            _make_test_file(fname)
            loc = ihm.location.InputFileLocation(fname)
            self.assertEqual(loc.path, os.path.abspath(fname))
            self.assertIsNone(loc.repo)
            self.assertEqual(loc.file_size, 8)

    def test_file_location_local_not_exist(self):
        """Test InputFileLocation with a local file that doesn't exist"""
        with utils.temporary_directory() as tmpdir:
            fname = os.path.join(tmpdir, 'test.pdb')
            self.assertRaises(
                ValueError, ihm.location.InputFileLocation, fname)

    def test_file_location_repo(self):
        """Test InputFileLocation with a file in a repository"""
        r = ihm.location.Repository(doi='1.2.3.4')
        loc = ihm.location.InputFileLocation('foo/bar', repo=r)
        self.assertEqual(loc.path, 'foo/bar')
        self.assertEqual(loc.repo, r)
        self.assertIsNone(loc.file_size)
        # locations should only compare equal if path and repo both match
        loc2 = ihm.location.InputFileLocation('foo/bar', repo=r)
        self.assertEqual(loc, loc2)
        loc3 = ihm.location.InputFileLocation('otherpath', repo=r)
        self.assertNotEqual(loc, loc3)
        r2 = ihm.location.Repository(doi='5.6.7.8')
        loc4 = ihm.location.InputFileLocation('foo/bar', repo=r2)
        self.assertNotEqual(loc, loc4)
        loc5 = ihm.location.InputFileLocation(None, repo=r)
        self.assertNotEqual(loc, loc5)
        loc6 = ihm.location.InputFileLocation(None, repo=r2)
        self.assertNotEqual(loc, loc6)

    def test_repository_equality(self):
        """Test Repository equality"""
        r1 = ihm.location.Repository(doi='foo')
        r2 = ihm.location.Repository(doi='foo')
        r3 = ihm.location.Repository(doi='foo', url='bar')
        r4 = ihm.location.Repository(doi='bar')
        self.assertEqual(r1, r2)
        self.assertEqual(hash(r1), hash(r2))
        self.assertNotEqual(r1, r3)
        self.assertNotEqual(r1, r4)

    def test_repository(self):
        """Test Repository"""
        # Make tmpdir under current directory, as it's not always possible to
        # get a relative path from cwd to /tmp (e.g. on Windows where they may
        # be on different drives)
        with utils.temporary_directory(os.getcwd()) as tmpdir:
            subdir = os.path.join(tmpdir, 'subdir')
            subdir2 = os.path.join(tmpdir, 'subdir2')
            os.mkdir(subdir)
            _make_test_file(os.path.join(subdir, 'bar'))
            s = ihm.location.Repository(doi='10.5281/zenodo.46266',
                                        root=os.path.relpath(tmpdir),
                                        url='foo', top_directory='baz')
            self.assertEqual(s._root, tmpdir)
            self.assertEqual(s.url, 'foo')
            self.assertEqual(s.top_directory, 'baz')

            loc = ihm.location.InputFileLocation(
                os.path.relpath(os.path.join(subdir, 'bar')))
            self.assertIsNone(loc.repo)
            ihm.location.Repository._update_in_repos(loc, [s])
            self.assertEqual(loc.repo.doi, '10.5281/zenodo.46266')
            self.assertEqual(loc.path, os.path.join('subdir', 'bar'))

            # Shouldn't touch locations that are already in repos
            loc = ihm.location.InputFileLocation(repo='foo', path='bar')
            self.assertEqual(loc.repo, 'foo')
            ihm.location.Repository._update_in_repos(loc, [s])
            self.assertEqual(loc.repo, 'foo')

            # Shortest match should win
            loc = ihm.location.InputFileLocation(
                os.path.relpath(os.path.join(subdir, 'bar')))
            s2 = ihm.location.Repository(doi='10.5281/zenodo.46280',
                                         root=os.path.relpath(subdir),
                                         url='foo', top_directory='baz')
            # Repositories that aren't above the file shouldn't count
            s3 = ihm.location.Repository(doi='10.5281/zenodo.56280',
                                         root=os.path.relpath(subdir2),
                                         url='foo', top_directory='baz')
            ihm.location.Repository._update_in_repos(loc, [s2, s3, s])
            self.assertEqual(loc.repo.doi, '10.5281/zenodo.46280')
            self.assertEqual(loc.path, 'bar')

    def test_repository_no_checkout(self):
        """Test Repository with no checkout"""
        r = ihm.location.Repository(doi='10.5281/zenodo.46266')
        f = ihm.location.InputFileLocation(repo=r, path='foo')
        self.assertEqual(f.repo.doi, '10.5281/zenodo.46266')
        self.assertEqual(f.path, 'foo')

    def test_repository_get_full_path(self):
        """Test Repository._get_full_path"""
        r = ihm.location.Repository(doi='10.5281/zenodo.46266',
                                    top_directory='/foo')
        self.assertEqual(r._get_full_path('bar'), '/foo%sbar' % os.sep)

    def test_file_locations(self):
        """Test FileLocation derived classes"""
        r = ihm.location.Repository(doi='10.5281/zenodo.46266')
        loc = ihm.location.InputFileLocation(repo=r, path='foo')
        self.assertEqual(loc.content_type, 'Input data or restraints')
        loc = ihm.location.OutputFileLocation(repo=r, path='foo')
        self.assertEqual(loc.content_type,
                         'Modeling or post-processing output')
        loc = ihm.location.WorkflowFileLocation(repo=r, path='foo')
        self.assertEqual(loc.content_type, 'Modeling workflow or script')
        loc = ihm.location.VisualizationFileLocation(repo=r, path='foo')
        self.assertEqual(loc.content_type, 'Visualization script')


if __name__ == '__main__':
    unittest.main()
