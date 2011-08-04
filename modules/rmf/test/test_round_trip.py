import unittest
import IMP.rmf
import IMP.test
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
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
        f= IMP.rmf.create_rmf_file(self.get_tmp_file_name("test_rt.rmf"))
        IMP.rmf.add_hierarchy(f, h)
        print "reopening"
        del f
        f= IMP.rmf.open_rmf_file_read_only(self.get_tmp_file_name("test_rt.rmf"))
        print "reading"
        h2=IMP.rmf.create_hierarchies(f, m)
        print "checking"
        self.assertEqual(len(h2), 1)
        self.assertEqual(len(IMP.atom.get_leaves(h)),
                         len(IMP.atom.get_leaves(h2[0])))
        self.assertAlmostEqual(IMP.atom.get_mass(h),
                               IMP.atom.get_mass(h2[0]), delta=1e-4)
        if hasattr(IMP.atom, 'get_surface_area'):
            self.assertAlmostEqual(IMP.atom.get_surface_area(h),
                                   IMP.atom.get_surface_area(h2[0]), delta=1e-4)
            self.assertAlmostEqual(IMP.atom.get_volume(h),
                                   IMP.atom.get_volume(h2[0]), delta=1e-4)


    def test_part1(self):
        """Test round trip 1"""
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
        f= IMP.rmf.create_rmf_file(self.get_tmp_file_name("test_rt_parts.rmf"))
        IMP.rmf.add_hierarchy(f, h)
    def test_part2(self):
        """Test round trip 2"""
        m= IMP.Model()
        print "reopening"
        f= IMP.rmf.open_rmf_file_read_only(self.get_tmp_file_name("test_rt_parts.rmf"))
        print "reading"
        h2=IMP.rmf.create_hierarchies(f, m)

if __name__ == '__main__':
    unittest.main()
