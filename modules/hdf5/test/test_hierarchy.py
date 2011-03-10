import unittest
import IMP.hdf5
import IMP.test
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test writing a simple hierarchy"""
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("small.pdb"), m)
        IMP.set_log_level(IMP.SILENT)
        IMP.atom.add_bonds(h)
        IMP.set_log_level(IMP.VERBOSE)
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("test_small_pdb.rh"), True)
        IMP.hdf5.add_hierarchy(h, f)
    def test_huge(self):
        """Test writing a huge hierarchy"""
        m= IMP.Model()
        print "reading pdb"
        h= IMP.atom.read_pdb(self.get_input_file_name("huge_protein.pdb"), m,
                             IMP.atom.NonAlternativePDBSelector())
        IMP.set_log_level(IMP.SILENT)
        IMP.atom.add_bonds(h)
        print "done"
        IMP.set_log_level(IMP.VERBOSE)
        print "writing hierarchy"
        IMP.set_log_level(IMP.PROGRESS)
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("test_huge.rh"), True)
        IMP.hdf5.add_hierarchy(h, f)
        print "done"
    def test_large(self):
        """Test writing a large hierarchy"""
        m= IMP.Model()
        print "reading pdb"
        h= IMP.atom.read_pdb(self.get_input_file_name("large.pdb"), m,
                             IMP.atom.NonAlternativePDBSelector())
        IMP.set_log_level(IMP.SILENT)
        IMP.atom.add_bonds(h)
        print "done"
        IMP.set_log_level(IMP.VERBOSE)
        print "writing hierarchy"
        IMP.set_log_level(IMP.PROGRESS)
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("test_large.rh"), True)
        IMP.hdf5.add_hierarchy(h, f)
        print "done"

if __name__ == '__main__':
    unittest.main()
