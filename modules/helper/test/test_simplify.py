import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class DecoratorTests(IMP.test.TestCase):
    def test_simplify_2(self):
        """Test protein simplification 2"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.atom.read_pdb(self.get_input_file_name('single_protein.pdb'), m)
        #IMP.atom.show_molecular_hierarchy(p)
        IMP.atom.add_radii(p)
        s= IMP.helper.create_simplified(p, 20)
        ls= IMP.core.get_leaves(s)
        for q in []:
            d= IMP.core.XYZR(q.get_particle())
            print ".sphere " +str(d.get_coordinates()[0]) + " "\
                + str(d.get_coordinates()[1]) + " "\
                + str(d.get_coordinates()[2]) + " "\
                + str(d.get_radius())
        print "level is " +str(IMP.get_log_level())

if __name__ == '__main__':
    unittest.main()
