from __future__ import print_function, division
import IMP.test
import IMP.pmi
import IMP.pmi.topology
import os

class Tests(IMP.test.TestCase):

    def test_sequences(self):
        """Test reading sequences from a FASTA file"""
        s = IMP.pmi.topology.Sequences(self.get_input_file_name('mini.fasta'))
        self.assertEqual(len(s), 2)
        self.assertIn('1WCM:A', s)
        self.assertNotIn('1WCM:X', s)
        self.assertEqual(s['1WCM:A'], 'MVGQQYSS')
        self.assertEqual(s[0], 'MVGQQYSS')
        self.assertRaises(IndexError, lambda x: x[4], s)
        # exercise __repr__
        x = repr(s)

    def test_sequences_name_map(self):
        """Test reading sequences from a FASTA file with mapped names"""
        nm = {'1WCM:A':'foo'}
        s = IMP.pmi.topology.Sequences(self.get_input_file_name('mini.fasta'),
                                       name_map=nm)
        self.assertIn('1WCM:B', s)
        self.assertNotIn('1WCM:A', s)
        self.assertEqual(s['foo'], 'MVGQQYSS')

    def test_sequences_uniprot(self):
        """Test reading sequences with UniProt info from a FASTA file"""
        s = IMP.pmi.topology.Sequences(
            self.get_input_file_name('uniprot.fasta'))
        self.assertIn('test1', s)
        self.assertIn('test2', s)
        self.assertEqual(s.uniprot['test1'], 'acc1')
        self.assertEqual(s.uniprot['test2'], 'acc2')

    def test_sequences_bad(self):
        """Test reading sequences from an invalid FASTA file"""
        fname = 'test_sequences_bad.fasta'
        with open('test_sequences_bad.fasta', 'w') as fh:
            fh.write('CCC*\n>FOO\nCCC*')
        self.assertRaises(Exception, IMP.pmi.topology.Sequences, fname)
        os.unlink(fname)

    def test_sequences_empty(self):
        """Test reading sequences from an empty FASTA file"""
        fname = 'test_sequences_empty.fasta'
        with open('test_sequences_empty.fasta', 'w') as fh:
            fh.write('\n\n')
        s = IMP.pmi.topology.Sequences(fname)
        self.assertEqual(len(s), 0)
        os.unlink(fname)


if __name__ == '__main__':
    IMP.test.main()
