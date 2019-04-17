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
        self.assertEqual(dl1.details, None)

    def test_pdb_location(self):
        """Test PDBLocation"""
        l = ihm.location.PDBLocation('1abc', version='foo', details='bar')
        self.assertEqual(l.db_name, 'PDB')
        self.assertEqual(l.access_code, '1abc')
        self.assertEqual(l.version, 'foo')
        self.assertEqual(l.details, 'bar')

    def test_emdb_location(self):
        """Test EMDBLocation"""
        l = ihm.location.EMDBLocation('EMDB-123', version='foo', details='bar')
        self.assertEqual(l.db_name, 'EMDB')
        self.assertEqual(l.access_code, 'EMDB-123')
        self.assertEqual(l.version, 'foo')
        self.assertEqual(l.details, 'bar')

    def test_massive_location(self):
        """Test MassIVELocation class"""
        d = ihm.location.MassIVELocation('abc', version=1, details='foo')
        self.assertEqual(d.db_name, 'MASSIVE')
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
        l = ihm.location.Location(details='foo')
        l._allow_duplicates = True
        self.assertEqual(l._eq_vals(), id(l))
        # Locations should never compare equal to None
        self.assertNotEqual(l, None)

    def test_file_location_local(self):
        """Test InputFileLocation with a local file"""
        # Make tmpdir under current directory, as it's not always possible to
        # get a relative path from cwd to /tmp (e.g. on Windows where they may
        # be on different drives)
        with utils.temporary_directory('.') as tmpdir:
            fname = os.path.join(tmpdir, 'test.pdb')
            _make_test_file(fname)
            l = ihm.location.InputFileLocation(fname)
            self.assertEqual(l.path, os.path.abspath(fname))
            self.assertEqual(l.repo, None)
            self.assertEqual(l.file_size, 8)

    def test_file_location_local_not_exist(self):
        """Test InputFileLocation with a local file that doesn't exist"""
        with utils.temporary_directory() as tmpdir:
            fname = os.path.join(tmpdir, 'test.pdb')
            self.assertRaises(ValueError, ihm.location.InputFileLocation, fname)

    def test_file_location_repo(self):
        """Test InputFileLocation with a file in a repository"""
        r = ihm.location.Repository(doi='1.2.3.4')
        l = ihm.location.InputFileLocation('foo/bar', repo=r)
        self.assertEqual(l.path, 'foo/bar')
        self.assertEqual(l.repo, r)
        self.assertEqual(l.file_size, None)
        # locations should only compare equal if path and repo both match
        l2 = ihm.location.InputFileLocation('foo/bar', repo=r)
        self.assertEqual(l, l2)
        l3 = ihm.location.InputFileLocation('otherpath', repo=r)
        self.assertNotEqual(l, l3)
        r2 = ihm.location.Repository(doi='5.6.7.8')
        l4 = ihm.location.InputFileLocation('foo/bar', repo=r2)
        self.assertNotEqual(l, l4)
        l5 = ihm.location.InputFileLocation(None, repo=r)
        self.assertNotEqual(l, l5)
        l6 = ihm.location.InputFileLocation(None, repo=r2)
        self.assertNotEqual(l, l6)

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
            self.assertEqual(loc.repo, None)
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
        l = ihm.location.InputFileLocation(repo=r, path='foo')
        self.assertEqual(l.content_type, 'Input data or restraints')
        l = ihm.location.OutputFileLocation(repo=r, path='foo')
        self.assertEqual(l.content_type, 'Modeling or post-processing output')
        l = ihm.location.WorkflowFileLocation(repo=r, path='foo')
        self.assertEqual(l.content_type, 'Modeling workflow or script')
        l = ihm.location.VisualizationFileLocation(repo=r, path='foo')
        self.assertEqual(l.content_type, 'Visualization script')


if __name__ == '__main__':
    unittest.main()
