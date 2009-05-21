import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_simplify(self):
        """Test protein simplification"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.atom.read_pdb(self.get_input_file_name('single_protein.pdb'), m)
        IMP.helper.simplified(p, 30)


if __name__ == '__main__':
    unittest.main()
