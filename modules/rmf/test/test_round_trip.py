import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *


class Tests(IMP.test.TestCase):

    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print(i, g.get_child_name(i), g.get_child_is_group(i))

    def test_rt(self):
        """Test round trip"""
        for suffix in [".rmfz", ".rmf3"]:
            m = IMP.Model()
            print("reading pdb")
            name = self.get_tmp_file_name("test_round_trip" + suffix)
            h = IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                  IMP.atom.NonAlternativePDBSelector())
            chain = h.get_child(0)
            self.assertTrue(IMP.atom.Chain.get_is_setup(chain))
            chain = IMP.atom.Chain(chain)
            chain.set_uniprot_accession('foobar')
            chain.set_sequence_offset(-10)
            chain.set_label_asym_id('AAA')
            h.get_is_valid(True)
            IMP.set_log_level(IMP.SILENT)
            IMP.atom.add_bonds(h)
            print("done")
            print("writing hierarchy")
            f = RMF.create_rmf_file(name)
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.save_frame(f, str(0))
            print("reopening")
            del f
            print("after closing")
        # print RMF.get_open_hdf5_handle_names()
            f = RMF.open_rmf_file_read_only(name)
            print("reading")
            print("creating")
            h2 = IMP.rmf.create_hierarchies(f, m)
            print("output")
            IMP.atom.show_molecular_hierarchy(h2[0])
            print("loading")
            IMP.rmf.load_frame(f, RMF.FrameID(0))
            print("output")
            IMP.atom.show_molecular_hierarchy(h2[0])
            print("checking")
            self.assertEqual(len(h2), 1)
            self.assertEqual(len(IMP.atom.get_leaves(h)),
                             len(IMP.atom.get_leaves(h2[0])))
            self.assertEqual(
                len([a for a in IMP.atom.get_leaves(h2)
                     if not IMP.atom.Atom.get_is_setup(a)]),
                0)
            self.assertAlmostEqual(IMP.atom.get_mass(IMP.atom.Selection(h)),
                                   IMP.atom.get_mass(IMP.atom.Selection(h2[0])), delta=1e-4)
            if hasattr(IMP.atom, 'get_surface_area'):
                self.assertAlmostEqual(IMP.atom.get_surface_area(h),
                                       IMP.atom.get_surface_area(h2[0]), delta=1e-4)
                self.assertAlmostEqual(IMP.atom.get_volume(h),
                                       IMP.atom.get_volume(h2[0]), delta=1e-4)
            chain = h2[0].get_child(0)
            self.assertTrue(IMP.atom.Chain.get_is_setup(chain))
            chain = IMP.atom.Chain(chain)
            # Accession and sequence offset requires RMF >= 1.6
            if hasattr(RMF.Chain, 'get_uniprot_accession'):
                self.assertEqual(chain.get_uniprot_accession(), 'foobar')
                self.assertEqual(chain.get_sequence_offset(), -10)
            # asym ID requires RMF >= 1.7
            if hasattr(RMF.Chain, 'get_label_asym_id'):
                self.assertEqual(chain.get_label_asym_id(), 'AAA')

    def test_part1(self):
        """Test round trip 1"""
        for suffix in [".rmfz", ".rmf3"]:
            m = IMP.Model()
            print("reading pdb")
            h = IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                  IMP.atom.NonAlternativePDBSelector())
            IMP.set_log_level(IMP.SILENT)
            IMP.atom.add_bonds(h)
            print("done")
            IMP.set_log_level(IMP.VERBOSE)
            print("writing hierarchy")
            name = self.get_tmp_file_name("test_rt_parts" + suffix)
            f = RMF.create_rmf_file(name)
            print(f, type(f))
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.save_frame(f, str(0))
            del f
            f = RMF.open_rmf_file_read_only(name)
            print("reading")
            print(f, type(f))
            h2 = IMP.rmf.create_hierarchies(f, m)
            self.assertEqual(len(h2), 1)
            del f

            m = IMP.Model()
            print("reopening")
            f = RMF.open_rmf_file_read_only(name)
            print("reading")
            h2 = IMP.rmf.create_hierarchies(f, m)

    def test_names(self):
        """Test if RMF can recall the molecule name and chain ID"""
        for suffix in [".rmfz", ".rmf3"]:
            for sequence in ('', 'CYW'):
                m = IMP.Model()
                h = IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                      IMP.atom.NonAlternativePDBSelector())
                chs = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
                chs[0].set_name('simple')
                IMP.atom.Chain(chs[0]).set_sequence(sequence)
                IMP.atom.Chain(chs[0]).set_chain_type(IMP.atom.Protein)
                self.assertEqual(chs[0].get_name(), 'simple')

                IMP.set_log_level(IMP.SILENT)
                IMP.atom.add_bonds(h)

                IMP.set_log_level(IMP.VERBOSE)
                name = self.get_tmp_file_name("test_rt_parts" + suffix)
                f = RMF.create_rmf_file(name)
                IMP.rmf.add_hierarchy(f, h)
                IMP.rmf.save_frame(f, str(0))
                del f
                f = RMF.open_rmf_file_read_only(name)
                h2 = IMP.rmf.create_hierarchies(f, m)
                del f
                chs2 = IMP.atom.get_by_type(h2[0], IMP.atom.CHAIN_TYPE)
                c = IMP.atom.Chain(chs2[0])
                self.assertEqual(c.get_id(), 'A')
                self.assertEqual(c.get_sequence(), sequence)
                # Protein should be an alias for LPolypeptide
                self.assertEqual(c.get_chain_type(), IMP.atom.Protein)
                self.assertEqual(c.get_chain_type(), IMP.atom.LPolypeptide)
                self.assertEqual(chs2[0].get_name(), 'simple')

    def test_references(self):
        """Test if Reference information is preserved in RMF"""
        for suffix in [".rmfz", ".rmf3"]:
            m = IMP.Model()
            h = IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                  IMP.atom.NonAlternativePDBSelector())
            rs = IMP.atom.get_by_type(h, IMP.atom.RESIDUE_TYPE)
            IMP.core.Reference.setup_particle(rs[1], rs[0])
            name = self.get_tmp_file_name("test_rt_refs" + suffix)
            f = RMF.create_rmf_file(name)
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.save_frame(f, str(0))
            del f
            f = RMF.open_rmf_file_read_only(name)
            h2 = IMP.rmf.create_hierarchies(f, m)
            rs2 = IMP.atom.get_by_type(h2[0], IMP.atom.RESIDUE_TYPE)
            self.assertFalse(IMP.core.Reference.get_is_setup(rs2[0]))
            self.assertTrue(IMP.core.Reference.get_is_setup(rs2[1]))
            r = IMP.core.Reference(rs2[1])
            self.assertEqual(r.get_reference_particle(), rs2[0])

if __name__ == '__main__':
    IMP.test.main()
