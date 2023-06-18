from __future__ import print_function
import IMP
import IMP.test
import IMP.pmi.alphabets


class Tests(IMP.test.TestCase):

    def test_amino_acid(self):
        """Test amino_acid alphabet"""
        alpha = IMP.pmi.alphabets.amino_acid
        self.assertEqual(alpha.get_chain_type(), IMP.atom.Protein)
        for code, restyp in [
            ('A', IMP.atom.ALA), ('R', IMP.atom.ARG), ('N', IMP.atom.ASN),
            ('D', IMP.atom.ASP), ('C', IMP.atom.CYS), ('E', IMP.atom.GLU),
            ('Q', IMP.atom.GLN), ('G', IMP.atom.GLY), ('H', IMP.atom.HIS),
            ('I', IMP.atom.ILE), ('L', IMP.atom.LEU), ('K', IMP.atom.LYS),
            ('M', IMP.atom.MET), ('F', IMP.atom.PHE), ('P', IMP.atom.PRO),
            ('S', IMP.atom.SER), ('T', IMP.atom.THR), ('W', IMP.atom.TRP),
            ('Y', IMP.atom.TYR), ('V', IMP.atom.VAL), ('X', IMP.atom.UNK)]:
            rt = alpha.get_residue_type_from_one_letter_code(code)
            self.assertEqual(rt, restyp)
            c = alpha.get_one_letter_code_from_residue_type(restyp.get_string())
            self.assertEqual(c, code)
        # Unknown type should always map to X
        c = alpha.get_one_letter_code_from_residue_type("GARBAGE")
        self.assertEqual(c, 'X')

    def test_dna(self):
        """Test DNA alphabet"""
        alpha = IMP.pmi.alphabets.dna
        self.assertEqual(alpha.get_chain_type(), IMP.atom.DNA)
        for code, restyp in [
            ('A', IMP.atom.DADE), ('U', IMP.atom.DURA), ('C', IMP.atom.DCYT),
            ('G', IMP.atom.DGUA), ('T', IMP.atom.DTHY), ('X', IMP.atom.UNK)]:
            rt = alpha.get_residue_type_from_one_letter_code(code)
            self.assertEqual(rt, restyp)

    def test_rna(self):
        """Test RNA alphabet"""
        alpha = IMP.pmi.alphabets.rna
        self.assertEqual(alpha.get_chain_type(), IMP.atom.RNA)
        for code, restyp in [
            ('A', IMP.atom.ADE), ('U', IMP.atom.URA), ('C', IMP.atom.CYT),
            ('G', IMP.atom.GUA), ('T', IMP.atom.THY), ('X', IMP.atom.UNK)]:
            rt = alpha.get_residue_type_from_one_letter_code(code)
            self.assertEqual(rt, restyp)


if __name__ == '__main__':
    IMP.test.main()
