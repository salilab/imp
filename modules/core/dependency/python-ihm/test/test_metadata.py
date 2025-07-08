import utils
import os
import unittest
import warnings
import urllib.request
import urllib.error
from io import StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.metadata

try:
    from ihm import _format
except ImportError:
    _format = None


class Tests(unittest.TestCase):

    def test_parser(self):
        """Test Parser base class"""
        p = ihm.metadata.Parser()
        p.parse_file(None)  # does nothing

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
            self.assertIsNone(dataset.location.repo)

    def test_mrc_parser_emdb_ok(self):
        """Test MRCParser pointing to an MRC in EMDB, no network errors"""
        def mock_urlopen(url, timeout=None):
            return StringIO(
                '{"admin": {"key_dates": {"map_release": "2011-04-21"},'
                '"title": "test details"}}')
        p = ihm.metadata.MRCParser()
        fname = utils.get_input_file_name(TOPDIR, 'emd_1883.map.mrc-header')
        d = p.parse_file(fname)
        self.assertEqual(list(d.keys()), ['dataset'])
        dataset = d['dataset']
        self.assertEqual(dataset.data_type, '3DEM volume')
        self.assertEqual(dataset.location.db_name, 'EMDB')
        self.assertEqual(dataset.location.access_code, 'EMD-1883')

        # Need to mock out urllib.request so we don't hit the network
        # (expensive) every time we test
        try:
            orig_urlopen = urllib.request.urlopen
            urllib.request.urlopen = mock_urlopen
            self.assertEqual(dataset.location.version, '2011-04-21')
            self.assertEqual(dataset.location.details, 'test details')
            dataset.location.version = 'my version'
            dataset.location.details = 'my details'
            self.assertEqual(dataset.location.version, 'my version')
            self.assertEqual(dataset.location.details, 'my details')
        finally:
            urllib.request.urlopen = orig_urlopen

    def test_mrc_parser_emdb_bad(self):
        """Test MRCParser pointing to an MRC in EMDB, with a network error"""
        def mock_urlopen(url, timeout=None):
            raise urllib.error.URLError("Mock network error")
        p = ihm.metadata.MRCParser()
        fname = utils.get_input_file_name(TOPDIR, 'emd_1883.map.mrc-header')
        d = p.parse_file(fname)
        self.assertEqual(list(d.keys()), ['dataset'])
        dataset = d['dataset']
        self.assertEqual(dataset.data_type, '3DEM volume')
        self.assertEqual(dataset.location.db_name, 'EMDB')
        self.assertEqual(dataset.location.access_code, 'EMD-1883')

        # Mock out urllib.request to raise an error
        try:
            orig_urlopen = urllib.request.urlopen
            urllib.request.urlopen = mock_urlopen
            with warnings.catch_warnings(record=True) as w:
                warnings.simplefilter("always")
                self.assertIsNone(dataset.location.version)
                self.assertEqual(dataset.location.details,
                                 'Electron microscopy density map')
        finally:
            urllib.request.urlopen = orig_urlopen
        self.assertEqual(len(w), 1)

    def test_mrc_parser_emdb_override(self):
        """Test MRCParser pointing to an MRC in EMDB with
           overridden metadata"""
        def mock_urlopen(url, timeout=None):
            raise ValueError("shouldn't be here")
        p = ihm.metadata.MRCParser()
        fname = utils.get_input_file_name(TOPDIR, 'emd_1883.map.mrc-header')
        d = p.parse_file(fname)
        self.assertEqual(list(d.keys()), ['dataset'])
        dataset = d['dataset']
        self.assertEqual(dataset.data_type, '3DEM volume')
        self.assertEqual(dataset.location.db_name, 'EMDB')
        self.assertEqual(dataset.location.access_code, 'EMD-1883')
        # Set version manually; should prevent network access below
        dataset.location.version = 'foo'

        # Mock out urllib.request to raise an error
        try:
            orig_urlopen = urllib.request.urlopen
            urllib.request.urlopen = mock_urlopen
            self.assertEqual(dataset.location.version, 'foo')
            self.assertEqual(dataset.location.details,
                             'Electron microscopy density map')
        finally:
            urllib.request.urlopen = orig_urlopen

    def _parse_pdb(self, fname):
        p = ihm.metadata.PDBParser()
        return p.parse_file(fname)

    def test_official_pdb(self):
        """Test PDBParser when given an official PDB"""
        p = self._parse_pdb(utils.get_input_file_name(TOPDIR, 'official.pdb'))
        self.assertEqual(p['templates'], {})
        self.assertEqual(len(p['metadata']), 1)
        self.assertEqual(p['metadata'][0].helix_id, '10')
        self.assertIsNone(p['script'])
        self._check_parsed_official_pdb(p, pdb_format=True)

    def test_bad_header(self):
        """Test PDBParser when given a non-official PDB with HEADER line"""
        pdbname = utils.get_input_file_name(TOPDIR, 'bad_header.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertEqual(p['software'], [])
        self.assertEqual(p['metadata'], [])
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')

    def test_derived_pdb(self):
        """Test PDBarser when given a file derived from a PDB"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_pdb.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertIsNone(p['script'])
        self.assertEqual(p['entity_source'], {})
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Experimental model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'MED7C AND MED21 STRUCTURES FROM PDB ENTRY 1YKH, '
                         'ROTATED AND TRANSLATED TO ALIGN WITH THE '
                         'MED4-MED9 MODEL')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '1YKH')
        self.assertIsNone(parent.location.version)
        self.assertIsNone(parent.location.details)

    def test_derived_comp_model(self):
        """Test PDBParser when given a file derived from a comparative model"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_model.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertIsNone(p['script'])
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
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
        """Test PDBParser when given a file derived from an
           integrative model"""
        pdbname = utils.get_input_file_name(TOPDIR, 'derived_int_model.pdb')
        p = self._parse_pdb(pdbname)
        self.assertEqual(p['templates'], {})
        self.assertIsNone(p['script'])
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Integrative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
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
        pdbname = utils.get_input_file_name(TOPDIR,
                                            'modeller_model_no_aln.pdb')
        p = self.check_modeller_model(pdbname)
        for templates in p['templates'].values():
            for t in templates:
                self.assertIsNone(t.alignment_file)

    def check_modeller_model(self, pdbname, cif=False):
        if cif:
            p = self._parse_cif(pdbname)
        else:
            p = self._parse_pdb(pdbname)
        dataset = p['dataset']
        self.assertEqual(sorted(p['templates'].keys()), ['A', 'B'])
        s1, s2 = p['templates']['A']
        s3, = p['templates']['B']
        self.assertEqual(s1.asym_id, 'C')
        self.assertEqual(s1.seq_id_range, (33, 424))
        self.assertEqual(s1.template_seq_id_range, (33, 424))
        self.assertAlmostEqual(s1.sequence_identity.value, 100.0, delta=0.1)
        self.assertEqual(
            s1.sequence_identity.denominator,
            ihm.startmodel.SequenceIdentityDenominator.SHORTER_LENGTH)
        self.assertEqual(s2.asym_id, 'G')
        self.assertEqual(s2.seq_id_range, (429, 488))
        self.assertEqual(s2.template_seq_id_range, (482, 551))
        self.assertAlmostEqual(s2.sequence_identity.value, 10.0, delta=0.1)
        self.assertEqual(
            s2.sequence_identity.denominator,
            ihm.startmodel.SequenceIdentityDenominator.SHORTER_LENGTH)
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        p1, p2, p3 = dataset.parents
        self.assertEqual(s1.dataset, p1)
        self.assertEqual(s2.dataset, p2)
        self.assertEqual(s3.dataset, p3)
        self.assertEqual(p1.data_type, 'Experimental model')
        self.assertEqual(p1.location.db_name, 'PDB')
        self.assertEqual(p1.location.access_code, '3JRO')
        self.assertIsNone(p1.location.version)
        self.assertIsNone(p1.location.details)
        self.assertEqual(p2.location.access_code, '3F3F')
        self.assertEqual(p3.location.access_code, '1ABC')
        s, = p['software']
        self.assertEqual(len(s.citation.authors), 2)
        self.assertEqual(s.name, 'MODELLER')
        if cif:
            self.assertEqual(s.version, '10.4')
            self.assertEqual(
                s.description,
                'Comparative modeling by satisfaction of spatial restraints, '
                'build 2023/10/23 11:26:12')
        else:
            self.assertEqual(s.version, '9.18')
            self.assertEqual(
                s.description,
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
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '4BZK')
        self.assertIsNone(parent.location.version)
        self.assertIsNone(parent.location.details)
        s, = p['software']
        self.assertEqual(s.name, 'Phyre2')
        self.assertEqual(s.version, '2.0')

    def test_swiss_model_monomer(self):
        """Test PDBParser when given a SWISS-MODEL monomer model."""
        pdbname = utils.get_input_file_name(TOPDIR, 'swiss_model.pdb')
        p = self._parse_pdb(pdbname)
        s, = p['software']
        self.assertEqual(s.name, 'SWISS-MODEL')
        self.assertIn('using PROMOD3 engine', s.description)
        self.assertEqual(s.version, '1.3.0')
        s, = p['templates']['B']
        self.assertEqual(s.asym_id, 'B')
        self.assertEqual(s.seq_id_range, (15, 244))
        self.assertEqual(s.template_seq_id_range, (1, 229))
        self.assertAlmostEqual(s.sequence_identity.value, 40.35, delta=0.1)
        denom = ihm.startmodel.SequenceIdentityDenominator
        self.assertEqual(
            s.sequence_identity.denominator, denom.NUM_ALIGNED_WITHOUT_GAPS)
        # alignment is also stored in the PDB file
        self.assertEqual(s.alignment_file.path, pdbname)
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '3j9w')
        self.assertIsNone(parent.location.version)
        self.assertIsNone(parent.location.details)

    def test_swiss_model_multimer(self):
        """Test PDBParser when given a SWISS-MODEL multimer model."""
        pdbname = utils.get_input_file_name(TOPDIR, 'swiss_model_multimer.pdb')
        p = self._parse_pdb(pdbname)
        s, = p['software']
        self.assertEqual(s.name, 'SWISS-MODEL')
        self.assertIn('using PROMOD3 engine', s.description)
        self.assertEqual(s.version, '2.0.0')
        self.assertEqual(sorted(p['templates'].keys()), ['A', 'B', 'C', 'D'])
        s, = p['templates']['C']
        self.assertEqual(s.asym_id, 'C')
        self.assertEqual(s.seq_id_range, (14, 1356))
        self.assertEqual(s.template_seq_id_range, (8, 1340))
        self.assertAlmostEqual(s.sequence_identity.value, 40.95, delta=0.1)
        denom = ihm.startmodel.SequenceIdentityDenominator
        self.assertEqual(
            s.sequence_identity.denominator, denom.NUM_ALIGNED_WITHOUT_GAPS)
        # alignment is also stored in the PDB file
        self.assertEqual(s.alignment_file.path, pdbname)
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertEqual(dataset.location.path, pdbname)
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')
        parent, = dataset.parents
        self.assertEqual(parent.data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '6flq')
        self.assertIsNone(parent.location.version)
        self.assertIsNone(parent.location.details)

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
        self.assertIsNone(dataset.location.repo)
        self.assertEqual(dataset.location.details,
                         'Starting model structure')

    def test_get_aligned_region_empty(self):
        """Test _get_aligned_region() with empty alignment"""
        self.assertRaises(ValueError, ihm.metadata._get_aligned_region,
                          'AAAA', '----')

    def _parse_cif(self, fname):
        p = ihm.metadata.CIFParser()
        return p.parse_file(fname)

    def test_cif_official_pdb(self):
        """Test CIFParser when given an mmCIF in the official PDB database"""
        p = self._parse_cif(utils.get_input_file_name(TOPDIR, 'official.cif'))
        self._check_parsed_official_pdb(p)

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_binary_cif_official_pdb(self):
        """Test BinaryCIFParser when given a BinaryCIF in the official PDB"""
        fname = utils.get_input_file_name(TOPDIR, 'official.bcif')
        parser = ihm.metadata.BinaryCIFParser()
        p = parser.parse_file(fname)
        self._check_parsed_official_pdb(p)

    def _check_parsed_official_pdb(self, p, pdb_format=False):
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Experimental model')
        self.assertEqual(dataset.location.db_name, 'PDB')
        self.assertEqual(dataset.location.access_code, '2HBJ')
        if pdb_format:
            self.assertEqual(dataset.location.version, '14-JUN-06')
        else:
            self.assertEqual(dataset.location.version, '2021-11-10')
        details = ('Structure of the yeast nuclear exosome component, '
                   'Rrp6p, reveals an interplay between the active '
                   'site and the HRDC domain')
        if pdb_format:
            details = details.upper()
        self.assertEqual(dataset.location.details, details)

        es = p['entity_source']
        self.assertEqual(sorted(es.keys()), ['A', 'B', 'C', 'D'])
        self.assertEqual(es['B'], es['C'])
        self.assertEqual(es['A'].src_method, 'man')
        self.assertEqual(es['A'].gene.scientific_name, 'MUS MUSCULUS')
        self.assertEqual(es['A'].gene.common_name, 'HOUSE MOUSE')
        self.assertEqual(es['A'].gene.strain, 'TEST STRAIN 1')
        self.assertEqual(es['A'].gene.ncbi_taxonomy_id, '10090')
        self.assertEqual(es['A'].host.scientific_name, 'ESCHERICHIA COLI')
        self.assertEqual(es['A'].host.common_name, 'TEST COMMON 1')
        self.assertEqual(es['A'].host.ncbi_taxonomy_id, '562')
        self.assertEqual(es['A'].host.strain, 'TEST STRAIN 2')
        self.assertEqual(es['B'].src_method, 'nat')
        self.assertEqual(es['B'].scientific_name, 'ESCHERICHIA COLI')
        self.assertEqual(es['B'].common_name, 'TEST COMMON 2')
        self.assertEqual(es['B'].ncbi_taxonomy_id, '562')
        self.assertEqual(es['B'].strain, 'TEST STRAIN 3')
        self.assertEqual(es['D'].src_method, 'syn')
        self.assertEqual(es['D'].scientific_name, 'HELIANTHUS ANNUUS')
        self.assertEqual(es['D'].common_name, 'COMMON SUNFLOWER')
        self.assertEqual(es['D'].ncbi_taxonomy_id, '4232')
        # _pdbx_entity_src_syn.strain is not used in current PDB entries
        if pdb_format:
            self.assertEqual(es['D'].strain, 'TEST STRAIN 4')
        else:
            self.assertIsNone(es['D'].strain)

    def test_cif_model_archive(self):
        """Test CIFParser when given an mmCIF in Model Archive"""
        p = self._parse_cif(utils.get_input_file_name(TOPDIR,
                                                      'modarchive.cif'))
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'De Novo model')
        self.assertEqual(dataset.location.db_name, 'MODEL ARCHIVE')
        self.assertEqual(dataset.location.access_code, 'ma-bak-cepc-0250')
        self.assertEqual(dataset.location.version, '2022-11-30')
        self.assertEqual(dataset.location.details,
                         'Predicted interaction between CWP1 and IKI1')

    def test_cif_unknown(self):
        """Test CIFParser when given an mmCIF not in a database"""
        fname = utils.get_input_file_name(TOPDIR, 'unknown_model.cif')
        p = self._parse_cif(fname)
        dataset = p['dataset']
        self.assertEqual(dataset.data_type, 'Comparative model')
        self.assertIsInstance(dataset.location, ihm.location.FileLocation)
        self.assertEqual(dataset.location.path, fname)
        self.assertEqual(dataset.location.details, 'Starting model structure')

    def test_cif_modeller_model_old(self):
        """Test CIFParser when given a Modeller model, old style"""
        fname = utils.get_input_file_name(TOPDIR, 'modeller_model.cif')
        p = self.check_modeller_model(fname, cif=True)
        aliname = utils.get_input_file_name(TOPDIR, 'modeller_model.ali')
        script = utils.get_input_file_name(TOPDIR, 'modeller_model.py')
        self.assertEqual(p['script'].path, script)
        for templates in p['templates'].values():
            for t in templates:
                self.assertEqual(t.alignment_file.path, aliname)

    def test_cif_modeller_incomplete_model(self):
        """Test CIFParser when given an incomplete Modeller model"""
        fname = utils.get_input_file_name(TOPDIR, 'modeller_incomplete.cif')
        p = self._parse_cif(fname)
        self.assertIsNone(p['script'])

    def test_cif_modeller_modelcif(self):
        """Test CIFParser when given a Modeller ModelCIF-compliant model"""
        # For new-style Modeller models, should read software info directly
        # from the _software table, not _exptl
        fname = utils.get_input_file_name(TOPDIR, 'modeller_modelcif.cif')
        p = self.check_modeller_model(fname, cif=True)
        aliname = utils.get_input_file_name(TOPDIR, 'modeller_model.ali')
        script = utils.get_input_file_name(TOPDIR, 'modeller_model.py')
        self.assertEqual(p['script'].path, script)
        for templates in p['templates'].values():
            for t in templates:
                self.assertEqual(t.alignment_file.path, aliname)

    def test_cif_modbase_modelcif(self):
        """Test CIFParser when given a ModBase ModelCIF-compliant model"""
        fname = utils.get_input_file_name(
            TOPDIR, 'modbase-model_e224ef5d7f96947a99dd618618021328.cif')
        p = self._parse_cif(fname)
        dataset = p['dataset']
        # ModBase isn't in IHMCIF's official set of databases, so should
        # be reported as "Other"
        self.assertEqual(dataset.location.db_name, 'Other')
        self.assertEqual(dataset.location.access_code,
                         'e224ef5d7f96947a99dd618618021328')
        self.assertEqual(dataset.location.details,
                         "ModBase database of comparative protein "
                         "structure models")
        self.assertEqual(sorted(p['templates'].keys()), ['A'])
        s1, = p['templates']['A']
        self.assertEqual(s1.asym_id, 'B')
        self.assertEqual(s1.seq_id_range, (1, 149))
        self.assertEqual(s1.template_seq_id_range, (18, 166))
        self.assertAlmostEqual(s1.sequence_identity.value, 99.0, delta=0.1)
        self.assertEqual(
            s1.sequence_identity.denominator,
            ihm.startmodel.SequenceIdentityDenominator.SHORTER_LENGTH)
        self.assertEqual(dataset.data_type, 'Comparative model')
        p1, = dataset.parents
        self.assertEqual(p1.data_type, 'Experimental model')
        self.assertEqual(p1.location.db_name, 'PDB')
        self.assertEqual(p1.location.access_code, '2fom')
        self.assertEqual([s.name for s in p['software']],
                         ['ModPipe', 'MODELLER', 'modbase_pdb_to_cif.py'])

    def test_cif_alphafold_modelcif(self):
        """Test CIFParser when given an AlphaFoldDB ModelCIF-compliant model"""
        fname = utils.get_input_file_name(TOPDIR, 'AF-O78126-F1-model_v4.cif')
        p = self._parse_cif(fname)
        dataset = p['dataset']
        self.assertEqual(dataset.location.db_name, 'AlphaFoldDB')
        self.assertEqual(dataset.location.access_code, 'AF-O78126-F1')
        self.assertEqual(dataset.location.details, "Starting model structure")
        self.assertEqual(dataset.data_type, 'De Novo model')
        self.assertEqual(len(dataset.parents), 4)
        p1, p2, p3, p4 = dataset.parents
        self.assertEqual(p1.data_type, 'Experimental model')
        self.assertEqual(p1.location.db_name, 'PDB')
        self.assertEqual(p1.location.access_code, '6ENY')
        self.assertEqual(p2.location.access_code, '6FGB')
        self.assertEqual(p3.location.access_code, '1MHC')
        self.assertEqual(p4.location.access_code, '1K5N')
        self.assertEqual([s.name for s in p['software']],
                         ['AlphaFold', 'dssp'])
        self.assertEqual(sorted(p['templates'].keys()), ['A'])
        s1, s2, s3, s4 = p['templates']['A']
        self.assertEqual(s1.asym_id, 'F')
        self.assertEqual(s1.seq_id_range, (None, None))
        self.assertEqual(s1.template_seq_id_range, (None, None))
        self.assertIsNone(s1.sequence_identity)


if __name__ == '__main__':
    unittest.main()
