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
        _ = ihm.reference.Reference()  # noop

    def test_sequence(self):
        """Test Sequence class"""
        s = ihm.reference.Sequence(
            db_name='testdb', db_code='testcode', accession='testacc',
            sequence='CCCG', details='foo')
        s.alignments.append(ihm.reference.Alignment(
            db_begin=10, db_end=30, entity_begin=20, entity_end=40))
        self.assertEqual(s.db_name, 'testdb')
        self.assertEqual(s.db_code, 'testcode')
        self.assertEqual(s.accession, 'testacc')
        self.assertEqual(s.sequence, 'CCCG')
        self.assertEqual(s.details, 'foo')
        a, = s._get_alignments()
        self.assertEqual(a.db_begin, 10)
        self.assertEqual(a.db_end, 30)
        self.assertEqual(a.entity_begin, 20)
        self.assertEqual(a.entity_end, 40)
        self.assertEqual(a.seq_dif, [])

    def test_sequence_default_alignment(self):
        """Test Sequence class with default Alignment"""
        s = ihm.reference.Sequence(
            db_name='testdb', db_code='testcode',
            accession='testacc', sequence='CCCG', details='foo')
        self.assertEqual(s.db_name, 'testdb')
        self.assertEqual(s.db_code, 'testcode')
        self.assertEqual(s.accession, 'testacc')
        self.assertEqual(s.sequence, 'CCCG')
        self.assertEqual(s.details, 'foo')
        a1, = s._get_alignments()
        a1a, = s._get_alignments()
        # should get same default alignment each time (get cache 2nd time)
        self.assertEqual(id(a1), id(a1a))
        self.assertEqual(a1.db_begin, 1)
        self.assertIsNone(a1.db_end)
        self.assertEqual(a1.entity_begin, 1)
        self.assertIsNone(a1.entity_end)
        self.assertEqual(a1.seq_dif, [])

    def test_uniprot_sequence(self):
        """Test UniProtSequence class"""
        lpep = ihm.LPeptideAlphabet()
        sd = ihm.reference.SeqDif(seq_id=1, db_monomer=lpep['C'],
                                  monomer=lpep['W'], details='Test mutation')
        s = ihm.reference.UniProtSequence(
            db_code='testcode', accession='testacc', sequence='CCCG')
        s.alignments.append(ihm.reference.Alignment(seq_dif=[sd]))
        self.assertEqual(s.db_name, 'UNP')
        self.assertEqual(s.db_code, 'testcode')
        self.assertEqual(s.accession, 'testacc')
        self.assertEqual(s.sequence, 'CCCG')
        self.assertIsNone(s.details)
        a, = s.alignments
        self.assertEqual(len(a.seq_dif), 1)
        self.assertEqual(a.seq_dif[0].seq_id, 1)
        self.assertEqual(a.seq_dif[0].db_monomer.id, 'CYS')
        self.assertEqual(a.seq_dif[0].monomer.id, 'TRP')
        self.assertEqual(a.seq_dif[0].details, 'Test mutation')

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
        self.assertEqual(
            r.details,
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
