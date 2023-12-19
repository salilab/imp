from __future__ import print_function, division
import IMP.test
import IMP.pmi
import IMP.pmi.topology

class Tests(IMP.test.TestCase):

    def test_read_sequences_pdb(self):
        """Test reading sequences from a PDB file"""
        m = IMP.Model()
        s = IMP.pmi.topology.PDBSequences(m,
                                      self.get_input_file_name('prot.pdb'))
        self.check_pdb_sequences(s)

    def test_read_sequences_mmcif(self):
        """Test reading sequences from an mmCIF file"""
        m = IMP.Model()
        s = IMP.pmi.topology.PDBSequences(m,
                                      self.get_input_file_name('prot.cif'))
        self.check_pdb_sequences(s)

    def check_pdb_sequences(self, s):
        # Safe comparison since PDBSequences uses OrderedDict
        self.assertEqual(list(s.sequences.keys()), ['A', 'B', 'G'])
        a = s.sequences['A']
        self.assertEqual(list(a.items()),
                         [((55, 56), 'QE'), ((59, 63), 'VVKDL')])

if __name__ == '__main__':
    IMP.test.main()
