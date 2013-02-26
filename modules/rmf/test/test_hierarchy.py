import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def _assert_same(self, h0, h1):
        IMP.atom.get_mass(h0)
        print h1
        s=IMP.atom.Selection(h1)
        IMP.atom.get_mass(s)
        self.assertAlmostEqual(IMP.atom.get_mass(h0),
                               IMP.atom.get_mass(h1), delta=1)
        self.assertEqual(len(IMP.atom.get_leaves(h0)),
                         len(IMP.atom.get_leaves(h1)))
        self.assertEqual(len(IMP.atom.get_internal_bonds(h0)),
                         len(IMP.atom.get_internal_bonds(h1)))
    def _test_round_trip(self, h0, name):
        f= RMF.create_rmf_file(name)
        IMP.rmf.add_hierarchy(f, h0)
        IMP.rmf.save_frame(f, 0)
        del f
        f= RMF.open_rmf_file_read_only(name)
        h1= IMP.rmf.create_hierarchies(f, h0.get_model())
        IMP.rmf.load_frame(f, 0)
        self._assert_same(h0, h1[0])
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test writing a simple hierarchy"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            h= IMP.atom.read_pdb(self.get_input_file_name("small.pdb"), m)
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.atom.add_bonds(h)
            IMP.base.set_log_level(IMP.base.VERBOSE)
            self._test_round_trip(h, self.get_tmp_file_name("test_small_pdb."+suffix))

    # disable as it clobbers machines without much memory
    def _test_huge(self):
        """Test writing a huge hierarchy"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            print "reading pdb"
            h= IMP.atom.read_pdb(self.get_input_file_name("huge_protein.pdb"), m,
                                 IMP.atom.NonAlternativePDBSelector())
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.atom.add_bonds(h)
            print "done"
            IMP.base.set_log_level(IMP.base.VERBOSE)
            print "writing hierarchy"
            IMP.base.set_log_level(IMP.PROGRESS)
            self._test_round_trip(h, self.get_tmp_file_name("test_huge."+suffix))
            print "done"
    def test_large(self):
        """Test writing a large hierarchy"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            print "reading pdb"
            h= IMP.atom.read_pdb(self.get_input_file_name("large.pdb"), m,
                                 IMP.atom.NonAlternativePDBSelector())
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.atom.add_bonds(h)
            print "done"
            IMP.base.set_log_level(IMP.base.VERBOSE)
            print "writing hierarchy"
            IMP.base.set_log_level(IMP.PROGRESS)
            self._test_round_trip(h, self.get_tmp_file_name("test_large."+suffix))
            print "done"

    def test_navigation(self):
        """Test that navigation of read hierarchies works"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            print "reading pdb"
            h= IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                 IMP.atom.NonAlternativePDBSelector())
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.atom.add_bonds(h)
            name=self.get_tmp_file_name("test_large_nav."+suffix)
            f= RMF.create_rmf_file(name)
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.save_frame(f, 0)
            del f
            f= RMF.open_rmf_file_read_only(name)
            h1= IMP.rmf.create_hierarchies(f, m)
            IMP.rmf.load_frame(f, 0)
            res= IMP.atom.get_by_type(h1[0], IMP.atom.RESIDUE_TYPE)
            nres= IMP.atom.get_next_residue(IMP.atom.Residue(res[0]))

    def test_linking(self):
        """Test that linking hierarchies works"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            print "reading pdb"
            h= IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                 IMP.atom.NonAlternativePDBSelector())
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.atom.add_bonds(h)
            name=self.get_tmp_file_name("test_link."+suffix)
            f= RMF.create_rmf_file(name)
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.save_frame(f, 0);
            del f
            f= RMF.open_rmf_file_read_only(name)
            IMP.rmf.link_hierarchies(f, [h])
            IMP.rmf.load_frame(f, 0)
            res= IMP.atom.get_by_type(h, IMP.atom.RESIDUE_TYPE)
            nres= IMP.atom.get_next_residue(IMP.atom.Residue(res[0]))


if __name__ == '__main__':
    unittest.main()
