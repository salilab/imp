import utils
import os
import unittest
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.reference

class Tests(unittest.TestCase):

    def test_reference(self):
        """Test Reference base class"""
        r = ihm.reference.Reference()  # noop

    def test_sequence(self):
        """Test Sequence class"""
        s = ihm.reference.Sequence(db_name='testdb', db_code='testcode',
                accession='testacc', sequence='CCCG', align_begin=10,
                details='foo')
        self.assertEqual(s.db_name, 'testdb')
        self.assertEqual(s.db_code, 'testcode')
        self.assertEqual(s.accession, 'testacc')
        self.assertEqual(s.sequence, 'CCCG')
        self.assertEqual(s.align_begin, 10)
        self.assertEqual(s.details, 'foo')

    def test_uniprot_sequence(self):
        """Test UniProtSequence class"""
        s = ihm.reference.UniProtSequence(db_code='testcode',
                accession='testacc', sequence='CCCG')
        self.assertEqual(s.db_name, 'UNP')
        self.assertEqual(s.db_code, 'testcode')
        self.assertEqual(s.accession, 'testacc')
        self.assertEqual(s.sequence, 'CCCG')
        self.assertEqual(s.align_begin, 1)
        self.assertIsNone(s.details)

    def _get_from_uniprot_accession(self, fasta_fname):
        def mock_urlopen(url):
            self.assertTrue(url.endswith('/testacc.fasta'))
            fname = utils.get_input_file_name(TOPDIR, fasta_fname)
            return open(fname, 'rb')
        # Need to mock out urllib2 so we don't hit the network (expensive)
        # every time we test
        try:
            orig_urlopen = urllib2.urlopen
            urllib2.urlopen = mock_urlopen
            return ihm.reference.UniProtSequence.from_accession('testacc')
        finally:
            urllib2.urlopen = orig_urlopen

    def test_uniprot_sequence_from_accession(self):
        """Test UniProtSequence.from_accession()"""
        r = self._get_from_uniprot_accession('P52891.fasta')
        self.assertIsInstance(r, ihm.reference.UniProtSequence)
        self.assertEqual(r.db_code, 'NUP84_YEAST')
        self.assertEqual(r.accession, 'testacc')
        self.assertEqual(len(r.sequence), 726)
        self.assertEqual(r.sequence[:20], 'MELSPTYQTERFTKFSDTLK')
        self.assertEqual(r.details,
                'Nucleoporin NUP84 OS=Saccharomyces cerevisiae (strain ATCC '
                '204508 / S288c) OX=559292 GN=NUP84 PE=1 SV=1')

    def test_uniprot_sequence_from_accession_bad_header(self):
        """Test UniProtSequence.from_accession() with bad header"""
        self.assertRaises(ValueError, self._get_from_uniprot_accession,
                          'uniprot_bad_header.fasta')

    def test_uniprot_sequence_from_accession_no_details(self):
        """Test UniProtSequence.from_accession() with no details"""
        r = self._get_from_uniprot_accession('uniprot_no_details.fasta')
        self.assertIsInstance(r, ihm.reference.UniProtSequence)
        self.assertEqual(r.db_code, 'NUP84_YEAST')
        self.assertEqual(r.accession, 'testacc')
        self.assertEqual(len(r.sequence), 726)
        self.assertEqual(r.sequence[:20], 'MELSPTYQTERFTKFSDTLK')
        self.assertIsNone(r.details)


if __name__ == '__main__':
    unittest.main()
