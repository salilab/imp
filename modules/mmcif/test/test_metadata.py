from __future__ import print_function
import IMP.test
import IMP.mmcif.metadata
import IMP.mmcif.data
import ihm
try:
    import urllib.request as urlrequest
    import urllib.error as urlerror
except ImportError:
    import urllib2 as urlrequest
    urlerror = urlrequest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class MockSystem(object):
    def __init__(self):
        self.system = ihm.System()
        self._external_files = IMP.mmcif.data._ExternalFiles(self.system)
        self.datasets = IMP.mmcif.data._Datasets(self.system)
        self._software = IMP.mmcif.data._AllSoftware(self.system)


class Tests(IMP.test.TestCase):

    def test_gmm_parser_local_mrc(self):
        """Test GMMMetadataParser pointing to a locally-available MRC file"""
        system = MockSystem()
        p = IMP.mmcif.metadata._GMMParser()
        fname = self.get_input_file_name('test.gmm.txt')
        r = p.parse_file(fname)
        self.assertEqual(r['number_of_gaussians'], 20)
        self.assertEqual(r['dataset'].data_type, '3DEM volume')
        self.assertEqual(r['dataset'].location.path, fname)
        self.assertIs(r['dataset'].location.repo, None)
        parent, = r['dataset'].parents
        self.assertEqual(parent.data_type, '3DEM volume')
        self.assertEqual(parent.location.path,
                         self.get_input_file_name('Rpb8.mrc-header'))
        self.assertEqual(parent.location.details,
                         'Electron microscopy density map')
        self.assertIs(parent.location.repo, None)

    def test_gmm_parser_emdb(self):
        """Test GMMParser pointing to an MRC in EMDB"""
        def mock_urlopen(url, timeout=None):
            txt = '{"EMD-1883":[{"deposition":{"map_release_date":"2011-04-21"'\
                  ',"title":"test details"}}]}'
            return StringIO(txt)
        system = MockSystem()
        p = IMP.mmcif.metadata._GMMParser()
        fname = self.get_input_file_name('emd_1883.map.mrc.gmm.50.txt')
        r = p.parse_file(fname)
        self.assertEqual(r['number_of_gaussians'], 50)
        self.assertEqual(r['dataset'].data_type, '3DEM volume')
        self.assertEqual(r['dataset'].location.path, fname)
        self.assertIs(r['dataset'].location.repo, None)
        parent, = r['dataset'].parents
        self.assertEqual(parent.data_type, '3DEM volume')
        self.assertEqual(parent.location.db_name, 'EMDB')
        self.assertEqual(parent.location.access_code, 'EMD-1883')

        # Need to mock out urllib.request so we don't hit the network
        # (expensive) every time we test
        try:
            orig_urlopen = urlrequest.urlopen
            urlrequest.urlopen = mock_urlopen
            self.assertEqual(parent.location.version, '2011-04-21')
            self.assertEqual(parent.location.details, 'test details')
        finally:
            urlrequest.urlopen = orig_urlopen

if __name__ == '__main__':
    IMP.test.main()
