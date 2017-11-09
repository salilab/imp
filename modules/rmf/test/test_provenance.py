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
        script = IMP.core.ScriptProvenance.setup_particle(
                            m, IMP.Particle(m), "test.py")
        script.set_name("script provenance")
        IMP.core.add_provenance(m, h, script)

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
        prov = IMP.core.get_all_provenance(chain)
        struc, = prov
        self.assertEqual(struc.get_chain_id(), 'A')
        self.assertEqual(struc.get_residue_offset(), 0)

        # Check the provenance we added at the top level
        allprov = list(IMP.core.get_all_provenance(h))
        self.assertEqual(len(allprov), 6)

        clus = allprov[0]
        self.assertEqual(clus.get_number_of_members(), 10)

        filt = allprov[1]
        self.assertEqual(filt.get_method(), "Total score")
        self.assertAlmostEqual(filt.get_threshold(), 100.5, delta=1e-4)
        self.assertEqual(filt.get_number_of_frames(), 39)

        comb = allprov[2]
        self.assertEqual(comb.get_number_of_runs(), 4)
        self.assertEqual(comb.get_number_of_frames(), 27)

        samp = allprov[3]
        self.assertEqual(samp.get_method(), "Monte Carlo")
        self.assertEqual(samp.get_number_of_frames(), 100)
        self.assertEqual(samp.get_number_of_iterations(), 42)

        struc = allprov[4]
        if sys.platform == 'win32':
            self.assertEqual(struc.get_filename(), "testfile")
        else:
            self.assertEqual(struc.get_filename(), os.path.abspath("testfile"))
        self.assertEqual(struc.get_chain_id(), "testchain")
        self.assertEqual(struc.get_residue_offset(), 19)
        self.assertEqual(struc.get_name(), "structure provenance")

        script = allprov[5]
        if sys.platform == 'win32':
            self.assertEqual(script.get_filename(), "test.py")
        else:
            self.assertEqual(script.get_filename(), os.path.abspath("test.py"))

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
