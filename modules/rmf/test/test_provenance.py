from __future__ import print_function
import unittest
import IMP.rmf
import IMP.test
import RMF
import os
import sys

class Tests(IMP.test.TestCase):

    def add_provenance(self, h):
        m = h.get_model()
        struc = IMP.core.StructureProvenance.setup_particle(
                            m, IMP.Particle(m), "testfile", "testchain", 19)
        struc.set_name("structure provenance")
        IMP.core.add_provenance(m, h, struc)

        samp = IMP.core.SampleProvenance.setup_particle(
                            m, IMP.Particle(m), "Monte Carlo", 100, 42)
        IMP.core.add_provenance(m, h, samp)

        comb = IMP.core.CombineProvenance.setup_particle(
                            m, IMP.Particle(m), 4, 27)
        IMP.core.add_provenance(m, h, comb)

        filt = IMP.core.FilterProvenance.setup_particle(
                            m, IMP.Particle(m), "Total score", 100.5, 39)
        IMP.core.add_provenance(m, h, filt)

        clus = IMP.core.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10)
        IMP.core.add_provenance(m, h, clus)

    def check_provenance(self, h):
        m = h.get_model()

        # Test IMP-added chain provenance
        chain, = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
        self.assertTrue(IMP.core.Provenanced.get_is_setup(m, chain))
        prov = IMP.core.Provenanced(m, chain).get_provenance()
        self.assertTrue(IMP.core.StructureProvenance.get_is_setup(m, prov))
        struc = IMP.core.StructureProvenance(m, prov)
        self.assertEqual(struc.get_chain_id(), 'A')
        self.assertEqual(struc.get_residue_offset(), 0)

        # Should be no more chain provenance
        prov = prov.get_previous()
        self.assertFalse(prov)

        # Check the provenance we added at the top level
        self.assertTrue(IMP.core.Provenanced.get_is_setup(m, h))
        prov = IMP.core.Provenanced(m, h).get_provenance()

        self.assertTrue(IMP.core.ClusterProvenance.get_is_setup(m, prov))
        clus = IMP.core.ClusterProvenance(m, prov)
        self.assertEqual(clus.get_number_of_members(), 10)

        prov = prov.get_previous()
        self.assertTrue(IMP.core.FilterProvenance.get_is_setup(m, prov))
        filt = IMP.core.FilterProvenance(m, prov)
        self.assertEqual(filt.get_method(), "Total score")
        self.assertAlmostEqual(filt.get_threshold(), 100.5, delta=1e-4)
        self.assertEqual(filt.get_number_of_frames(), 39)

        prov = prov.get_previous()
        self.assertTrue(IMP.core.CombineProvenance.get_is_setup(m, prov))
        comb = IMP.core.CombineProvenance(m, prov)
        self.assertEqual(comb.get_number_of_runs(), 4)
        self.assertEqual(comb.get_number_of_frames(), 27)

        prov = prov.get_previous()
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(m, prov))
        samp = IMP.core.SampleProvenance(m, prov)
        self.assertEqual(samp.get_method(), "Monte Carlo")
        self.assertEqual(samp.get_number_of_frames(), 100)
        self.assertEqual(samp.get_number_of_iterations(), 42)

        prov = prov.get_previous()
        self.assertTrue(IMP.core.StructureProvenance.get_is_setup(m, prov))
        struc = IMP.core.StructureProvenance(m, prov)
        if sys.platform == 'win32':
            self.assertEqual(struc.get_filename(), "testfile")
        else:
            self.assertEqual(struc.get_filename(), os.path.abspath("testfile"))
        self.assertEqual(struc.get_chain_id(), "testchain")
        self.assertEqual(struc.get_residue_offset(), 19)
        self.assertEqual(struc.get_name(), "structure provenance")

        # Should be no more provenance
        prov = prov.get_previous()
        self.assertFalse(prov)

    def test_rt(self):
        """Test that provenance info can be stored in RMF files"""
        for suffix in [".rmfz", ".rmf3"]:
            m = IMP.Model()
            name = self.get_tmp_file_name("test_provenance" + suffix)
            h = IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                  IMP.atom.NonAlternativePDBSelector())
            self.add_provenance(h)
            f = RMF.create_rmf_file(name)
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.save_frame(f, "0")
            del f
            f = RMF.open_rmf_file_read_only(name)
            h2 = IMP.rmf.create_hierarchies(f, m)
            self.check_provenance(h2[0])

if __name__ == '__main__':
    IMP.test.main()
