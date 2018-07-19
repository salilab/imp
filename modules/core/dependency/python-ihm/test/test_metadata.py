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

    def _parse_pdb(self, fname):
        p = ihm.metadata.PDBParser()
        return p.parse_file(fname)

    def test_official_pdb(self):
        """Test PDBParser when given an official PDB"""
        p = self._parse_pdb(utils.get_input_file_name(TOPDIR, 'official.pdb'))
        self.assertEqual(p['templates'], {})
        self.assertEqual(len(p['metadata']), 1)
        self.assertEqual(p['metadata'][0].helix_id, '10')
        self.assertEqual(p['script'], None)
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
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertEqual(p['script'], None)
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

    def test_derived_comp_model(self):
        """Test PDBParser when given a file derived from a comparative model"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_model.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertEqual(p['script'], None)
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

    def test_derived_int_model(self):
        """Test PDBParser when given a file derived from an integrative model"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_int_model.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertEqual(p['script'], None)
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Integrative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'POM152 STRUCTURE TAKEN FROM UPLA ET AL, STRUCTURE '
                         '25(3) 434-445. DOI: 10.1016/j.str.2017.01.006.')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Integrative model')
        self.assertEqual(parent.location.path, '.')
        self.assertEqual(parent.location.repo.doi, '10.1016/j.str.2017.01.006')
        self.assertEqual(parent.location.details,
                         'Starting integrative model structure')

    def test_modeller_model_aln(self):
        """Test PDBParser when given a Modeller model with alignment/script"""
        pdbname = utils.get_input_file_name(TOPDIR, 'modeller_model.pdb')
        p = self.check_modeller_model(pdbname)

        aliname = utils.get_input_file_name(TOPDIR, 'modeller_model.ali')
        script = utils.get_input_file_name(TOPDIR, 'modeller_model.py')
        self.assertEqual(p['script'].path, script)
        for templates in p['templates'].values():
            for t in templates:
                self.assertEqual(t.alignment_file.path, aliname)

    def test_modeller_model_no_aln(self):
        "Test PDBParser when given a Modeller model with no alignment/script"
        pdbname = utils.get_input_file_name(TOPDIR, 'modeller_model_no_aln.pdb')
        p = self.check_modeller_model(pdbname)
        for templates in p['templates'].values():
            for t in templates:
                self.assertEqual(t.alignment_file, None)

    def check_modeller_model(self, pdbname):
        p = self._parse_pdb(pdbname)
        dataset = p['dataset']
        self.assertEqual(sorted(p['templates'].keys()), ['A', 'B'])
        s1, s2 = p['templates']['A']
        s3, = p['templates']['B']
        self.assertEqual(s1.asym_id, 'C')
        self.assertEqual(s1.seq_id_range, (33,424))
        self.assertEqual(s1.template_seq_id_range, (33,424))
        self.assertAlmostEqual(float(s1.sequence_identity), 100.0, places=1)
        self.assertEqual(s1.sequence_identity.denominator,
                     ihm.startmodel.SequenceIdentityDenominator.SHORTER_LENGTH)
        self.assertEqual(s2.asym_id, 'G')
        self.assertEqual(s2.seq_id_range, (429,488))
        self.assertEqual(s2.template_seq_id_range, (482,551))
        self.assertAlmostEqual(float(s2.sequence_identity), 10.0, places=1)
        self.assertEqual(s2.sequence_identity.denominator,
                     ihm.startmodel.SequenceIdentityDenominator.SHORTER_LENGTH)
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        p1, p2, p3 = dataset.parents
        self.assertEqual(s1.dataset, p1)
        self.assertEqual(s2.dataset, p2)
        self.assertEqual(s3.dataset, p3)
        self.assertEqual(p1.data_type, 'Experimental model')
        self.assertEqual(p1.location.db_name, 'PDB')
        self.assertEqual(p1.location.access_code, '3JRO')
        self.assertEqual(p1.location.version, None)
        self.assertEqual(p1.location.details, None)
        self.assertEqual(p2.location.access_code, '3F3F')
        self.assertEqual(p3.location.access_code, '1ABC')
        s, = p['software']
        self.assertEqual(s.name, 'MODELLER')
        self.assertEqual(s.version, '9.18')
        self.assertEqual(s.description,
                 'Comparative modeling by satisfaction of spatial restraints, '
                 'build 2017/02/10 22:21:34')
        return p

    def test_modeller_local(self):
        "Test PDBParser when given a Modeller model with local template"
        pdbname = utils.get_input_file_name(TOPDIR, 'modeller_model_local.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(list(p['templates'].keys()), ['A'])
        s, = p['templates']['A']
        self.assertEqual(s.asym_id, 'C')
        parent, = p['dataset'].parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.details,
                         'Template for comparative modeling')
        self.assertEqual(parent.location.path,
                         utils.get_input_file_name(TOPDIR, '15133C.pdb'))

    def test_phyre2_model(self):
        """Test PDBParser when given a Phyre2 model."""
        pdbname = utils.get_input_file_name(TOPDIR, 'phyre2_model.pdb')
        p = self._parse_pdb(pdbname)
        s, = p['templates']['A']
        self.assertEqual(s.asym_id, 'A')
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
        s, = p['software']
        self.assertEqual(s.name, 'Phyre2')
        self.assertEqual(s.version, '2.0')

    def test_unknown_model(self):
        """Test PDBParser when given an unknown model."""
        pdbname = utils.get_input_file_name(TOPDIR, 'unknown_model.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertEqual(p['software'], [])
        self.assertEqual(p['metadata'], [])
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertEqual(dataset.location.repo, None)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')


if __name__ == '__main__':
    unittest.main()
