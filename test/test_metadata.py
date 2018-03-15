import utils
import os
import unittest
import sys
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.metadata

class Tests(unittest.TestCase):

    def test_parser(self):
        """Test Parser base class"""
        p = ihm.metadata.Parser()
        p.parse_file(None) # does nothing

    def test_mrc_parser_local_mrc(self):
        """Test MRCParser pointing to a locally-available MRC file"""
        p = ihm.metadata.MRCParser()
        # Note that this are not complete MRC files (only the headers),
        # to save space in the repository
        for input_file in ('Rpb8.mrc-header', 'emptylabel.mrc-header'):
            fname = utils.get_input_file_name(TOPDIR, input_file)
            d = p.parse_file(fname)
            self.assertEqual(list(d.keys()), ['dataset'])
            dataset = d['dataset']
            self.assertEqual(dataset.data_type, '3DEM volume')
            self.assertEqual(dataset.location.path, fname)
            self.assertEqual(dataset.location.details,
                             'Electron microscopy density map')
            self.assertEqual(dataset.location.repo, None)

    def test_mrc_parser_emdb(self):
        """Test MRCParser pointing to an MRC in EMDB"""
        def mock_urlopen(url):
            txt = '{"EMD-1883":[{"deposition":{"map_release_date":"2011-04-21"'\
                  ',"title":"test details"}}]}'
            return StringIO(txt)
        p = ihm.metadata.MRCParser()
        fname = utils.get_input_file_name(TOPDIR, 'emd_1883.map.mrc-header')

        # Need to mock out urllib2 so we don't hit the network (expensive)
        # every time we test
        try:
            orig_urlopen = urllib2.urlopen
            urllib2.urlopen = mock_urlopen
            d = p.parse_file(fname)
        finally:
            urllib2.urlopen = orig_urlopen
        self.assertEqual(list(d.keys()), ['dataset'])
        dataset = d['dataset']
        self.assertEqual(dataset.data_type, '3DEM volume')
        self.assertEqual(dataset.location.db_name, 'EMDB')
        self.assertEqual(dataset.location.access_code, 'EMD-1883')
        self.assertEqual(dataset.location.version, '2011-04-21')
        self.assertEqual(dataset.location.details, 'test details')

    def _parse_pdb(self, fname, chain):
        p = ihm.metadata.PDBParser()
        return p.parse_file(fname, chain)

    def test_official_pdb(self):
        """Test PDBParser when given an official PDB"""
        p = self._parse_pdb(utils.get_input_file_name(TOPDIR, 'official.pdb'),
                            'A')
        s, = p['sources']
        self.assertEqual(s.db_code, '2HBJ')
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(len(s.metadata), 1)
        self.assertEqual(s.metadata[0].helix_id, '10')
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Experimental model')
        self.assertEqual(dataset.location.db_name, 'PDB')
        self.assertEqual(dataset.location.access_code, '2HBJ')
        self.assertEqual(dataset.location.version, '14-JUN-06')
        self.assertEqual(dataset.location.details,
                         'STRUCTURE OF THE YEAST NUCLEAR EXOSOME COMPONENT, '
                         'RRP6P, REVEALS AN INTERPLAY BETWEEN THE ACTIVE '
                         'SITE AND THE HRDC DOMAIN')

    def test_derived_pdb(self):
        """Test PDBarser when given a file derived from a PDB"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_pdb.pdb')
        p = self._parse_pdb(pdbname, 'A')
        s, = p['sources']
        self.assertEqual(s.db_code, None)
        self.assertEqual(s.chain_id, 'A')
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Experimental model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'MED7C AND MED21 STRUCTURES FROM PDB ENTRY 1YKH, '
                         'ROTATED AND TRANSLATED TO ALIGN WITH THE '
                         'MED4-MED9 MODEL')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '1YKH')
        self.assertEqual(parent.location.version, None)
        self.assertEqual(parent.location.details, None)

    def test_derived_model(self):
        """Test PDBParser when given a file derived from a model"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_model.pdb')
        p = self._parse_pdb(pdbname, 'A')
        s, = p['sources']
        self.assertEqual(s.db_code, None)
        self.assertEqual(s.chain_id, 'A')
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'MED4 AND MED9 STRUCTURE TAKEN FROM LARIVIERE '
                         'ET AL, NUCLEIC ACIDS RESEARCH. 2013;41:9266-9273. '
                         'DOI: 10.1093/nar/gkt704. THE MED10 STRUCTURE ALSO '
                         'PROPOSED IN THAT WORK IS NOT USED IN THIS STUDY.')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Comparative model')
        self.assertEqual(parent.location.path, '.')
        self.assertEqual(parent.location.repo.doi, '10.1093/nar/gkt704')
        self.assertEqual(parent.location.details,
                         'Starting comparative model structure')

    def test_modeller_model_aln(self):
        """Test PDBParser when given a Modeller model with alignment"""
        pdbname = utils.get_input_file_name(TOPDIR, 'modeller_model.pdb')
        p = self.check_modeller_model(pdbname)

        aliname = utils.get_input_file_name(TOPDIR, 'modeller_model.ali')
        self.assertEqual(p['alignment'].path, aliname)

    def test_modeller_model_no_aln(self):
        "Test PDBParser when given a Modeller model with no alignment"
        pdbname = utils.get_input_file_name(TOPDIR, 'modeller_model_no_aln.pdb')
        p = self.check_modeller_model(pdbname)
        self.assertEqual(p['alignment'], None)

    def check_modeller_model(self, pdbname):
        p = self._parse_pdb(pdbname, 'A')
        s1, s2 = p['sources']
        self.assertEqual(s1.db_code, None)
        self.assertEqual(s1.chain_id, 'A')
        self.assertEqual(s1.tm_db_code, '3JRO')
        self.assertEqual(s1.tm_chain_id, 'C')
        self.assertEqual(s2.db_code, None)
        self.assertEqual(s2.chain_id, 'A')
        self.assertEqual(s2.tm_db_code, '3F3F')
        self.assertEqual(s2.tm_chain_id, 'G')
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        p1, p2 = dataset.parents
        self.assertEqual(p1.data_type, 'Experimental model')
        self.assertEqual(p1.location.db_name, 'PDB')
        self.assertEqual(p1.location.access_code, '3JRO')
        self.assertEqual(p1.location.version, None)
        self.assertEqual(p1.location.details, None)
        self.assertEqual(p2.location.access_code, '3F3F')
        self.assertEqual(p['software'],
                         {'modeller': ['9.18', '2017/02/10 22:21:34\n']})
        return p

    def test_modeller_local(self):
        "Test PDBParser when given a Modeller model with local template"
        pdbname = utils.get_input_file_name(TOPDIR, 'modeller_model_local.pdb')
        p = self._parse_pdb(pdbname, 'A')
        s, = p['sources']
        self.assertEqual(s.db_code, None)
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(s.tm_db_code, None)
        self.assertEqual(s.tm_chain_id, 'C')
        parent, = p['dataset'].parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.details,
                         'Template for comparative modeling')
        self.assertEqual(parent.location.path,
                         utils.get_input_file_name(TOPDIR, '15133C.pdb'))

    def test_phyre2_model(self):
        """Test PDBParser when given a Phyre2 model."""
        pdbname = utils.get_input_file_name(TOPDIR, 'phyre2_model.pdb')
        p = self._parse_pdb(pdbname, 'A')
        s, = p['sources']
        self.assertEqual(s.db_code, None)
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(s.tm_db_code, '4BZK')
        self.assertEqual(s.tm_chain_id, 'A')
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '4BZK')
        self.assertEqual(parent.location.version, None)
        self.assertEqual(parent.location.details, None)
        self.assertEqual(p['software'], {'phyre2': '?'})

    def test_unknown_model(self):
        """Test PDBParser when given an unknown model."""
        pdbname = utils.get_input_file_name(TOPDIR, 'unknown_model.pdb')
        p = self._parse_pdb(pdbname, 'A')
        s, = p['sources']
        self.assertEqual(s.db_code, None)
        self.assertEqual(s.chain_id, 'A')
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')


if __name__ == '__main__':
    unittest.main()
