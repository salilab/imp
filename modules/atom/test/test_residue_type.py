import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):
    """Test the ResidueType class and related functions"""

    def test_get_one_letter_type(self):
        """Check get residue type from one-letter code"""
        rt = IMP.atom.get_residue_type('A')
        self.assertEqual(rt.get_string(), 'ALA')
        rt = IMP.atom.get_residue_type('S')
        self.assertEqual(rt.get_string(), 'SER')
        # Unknown one-letter codes should raise an error
        self.assertRaises(ValueError, IMP.atom.get_residue_type, 'B')
        # One-letter codes are case-sensitive
        self.assertRaises(ValueError, IMP.atom.get_residue_type, 'a')

    def test_equality(self):
        """Check equality of ResidueType types"""
        at0= IMP.atom.ResidueType(0)
        at1= IMP.atom.ResidueType(0)
        at2= IMP.atom.ResidueType(1)
        self.assertEqual(at0, at1)
        self.assertNotEqual(at0, at2)

if __name__ == '__main__':
    IMP.test.main()
