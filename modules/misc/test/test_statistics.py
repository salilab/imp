import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.modeller
import math

class TunnelTest(IMP.test.TestCase):
    """Tests for tunnel scores"""

    def test_score(self):
        """Test printing usage"""
        m= IMP.Model()
        mh= IMP.modeller.read_pdb(self.get_input_file_name('single_protein.pdb'), m)
        IMP.misc.show_attribute_usage(m)

if __name__ == '__main__':
    unittest.main()
