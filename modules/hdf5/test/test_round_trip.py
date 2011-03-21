import unittest
import IMP.hdf5
import IMP.test
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_large(self):
        """Test round trip"""
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
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("test_rt.rh"), True)
        IMP.hdf5.write_hierarchy(h, f)
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("test_rt.rh"), False)
        h2=IMP.hdf5.read_all_hierarchies(f, m)
        self.assertEqual(len(h2), 1)
        self.assertEqual(len(IMP.atom.get_leaves(h)),
                         len(IMP.atom.get_leaves(h2[0])))
        self.assertAlmostEqual(IMP.atom.get_mass(h),
                               IMP.atom.get_mass(h2[0]), delta=1e-4)
        if IMP.atom.has_cgal:
            self.assertAlmostEqual(IMP.atom.get_surface_area(h),
                                   IMP.atom.get_surface_area(h2[0]), delta=1e-4)
            self.assertAlmostEqual(IMP.atom.get_volume(h),
                                   IMP.atom.get_volume(h2[0]), delta=1e-4)

if __name__ == '__main__':
    unittest.main()
