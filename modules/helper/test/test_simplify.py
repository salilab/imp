import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class DecoratorTests(IMP.test.TestCase):
    def test_simplify(self):
        """Test protein simplification"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.atom.read_pdb(self.get_input_file_name('single_protein.pdb'), m)
        s= IMP.helper.simplified(p, 10)
        ls= IMP.core.get_leaves(s)
        for q in ls:
            d= IMP.core.XYZR(q.get_particle())
            print ".sphere " +str(d.get_coordinates()[0]) + " "\
                + str(d.get_coordinates()[1]) + " "\
                + str(d.get_coordinates()[2]) + " "\
                + str(d.get_radius())


if __name__ == '__main__':
    unittest.main()
