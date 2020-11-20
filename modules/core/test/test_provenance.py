from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import sys
import os

class Tests(IMP.test.TestCase):

    def test_provenance(self):
        """Test Provenance decorator"""
        m = IMP.Model()
        p = IMP.core.Provenance.setup_particle(m, IMP.Particle(m))
        self.assertTrue(IMP.core.Provenance.get_is_setup(p))
        self.assertFalse(p.get_previous())

        p2 = IMP.core.Provenance.setup_particle(m, IMP.Particle(m))
        p.set_previous(p2)
        self.assertEqual(p.get_previous(), p2)

    def assertFilenameEqual(self, f1, f2):
        # Paths are stored internally as absolute paths (except on Windows)
        if sys.platform == 'win32':
            self.assertEqual(f1, f2)
        else:
            self.assertEqual(f1, os.path.abspath(f2))

    def test_structure_provenance(self):
        """Test StructureProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.StructureProvenance.setup_particle(m, IMP.Particle(m),
                               "testfile", "testchain")
        self.assertTrue(IMP.core.StructureProvenance.get_is_setup(p))
        self.assertFilenameEqual(p.get_filename(), "testfile")
        self.assertEqual(p.get_chain_id(), "testchain")
        self.assertEqual(p.get_residue_offset(), 0)
        p.set_residue_offset(42)
        self.assertEqual(p.get_residue_offset(), 42)

    def test_script_provenance(self):
        """Test ScriptProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.ScriptProvenance.setup_particle(m, IMP.Particle(m),
                                                     "testfile")
        self.assertTrue(IMP.core.ScriptProvenance.get_is_setup(p))
        self.assertFilenameEqual(p.get_filename(), "testfile")

    def test_software_provenance(self):
        """Test SoftwareProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.SoftwareProvenance.setup_particle(m, IMP.Particle(m),
                            "IMP", "1.0", "https://imp.test.org")
        self.assertTrue(IMP.core.SoftwareProvenance.get_is_setup(p))
        self.assertEqual(p.get_software_name(), "IMP")
        p.set_software_name('testname')
        self.assertEqual(p.get_software_name(), "testname")
        self.assertEqual(p.get_version(), "1.0")
        p.set_version('2.0')
        self.assertEqual(p.get_version(), "2.0")
        self.assertEqual(p.get_location(), "https://imp.test.org")
        p.set_location('foo')
        self.assertEqual(p.get_location(), "foo")

    def test_sample_provenance(self):
        """Test SampleProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.SampleProvenance.setup_particle(m, IMP.Particle(m),
                               "Monte Carlo", 100, 5)
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(p))
        self.assertEqual(p.get_method(), "Monte Carlo")
        p.set_method("Molecular Dynamics")
        self.assertEqual(p.get_method(), "Molecular Dynamics")
        self.assertRaisesUsageException(p.set_method, "Garbage")
        self.assertRaisesUsageException(
                 IMP.core.SampleProvenance.setup_particle, m, IMP.Particle(m),
                 "Garbage", 100, 5)
        self.assertEqual(p.get_number_of_frames(), 100)
        p.set_number_of_frames(200)
        self.assertEqual(p.get_number_of_frames(), 200)
        self.assertEqual(p.get_number_of_iterations(), 5)
        p.set_number_of_iterations(42)
        self.assertEqual(p.get_number_of_iterations(), 42)
        self.assertEqual(p.get_number_of_replicas(), 1)
        p.set_number_of_replicas(8)
        self.assertEqual(p.get_number_of_replicas(), 8)

    def test_combine_provenance(self):
        """Test CombineProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.CombineProvenance.setup_particle(m, IMP.Particle(m), 5, 42)
        self.assertTrue(IMP.core.CombineProvenance.get_is_setup(p))
        self.assertEqual(p.get_number_of_runs(), 5)
        p.set_number_of_runs(7)
        self.assertEqual(p.get_number_of_runs(), 7)
        self.assertEqual(p.get_number_of_frames(), 42)
        p.set_number_of_frames(100)
        self.assertEqual(p.get_number_of_frames(), 100)

    def test_filter_provenance(self):
        """Test FilterProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.FilterProvenance.setup_particle(m, IMP.Particle(m),
                                "Total score", 100.5, 42)
        self.assertTrue(IMP.core.FilterProvenance.get_is_setup(p))
        self.assertEqual(p.get_method(), "Total score")
        p.set_method("Keep fraction")
        self.assertEqual(p.get_method(), "Keep fraction")
        self.assertRaisesUsageException(p.set_method, "Garbage")
        self.assertRaisesUsageException(
                 IMP.core.FilterProvenance.setup_particle, m, IMP.Particle(m),
                 "Garbage", 100.5, 42)
        self.assertAlmostEqual(p.get_threshold(), 100.5, delta=0.01)
        p.set_threshold(76.0)
        self.assertAlmostEqual(p.get_threshold(), 76.0, delta=0.01)
        self.assertEqual(p.get_number_of_frames(), 42)
        p.set_number_of_frames(100)
        self.assertEqual(p.get_number_of_frames(), 100)

    def test_cluster_provenance(self):
        """Test ClusterProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10)
        self.assertTrue(IMP.core.ClusterProvenance.get_is_setup(p))
        self.assertEqual(p.get_number_of_members(), 10)
        p.set_number_of_members(42)
        self.assertEqual(p.get_number_of_members(), 42)
        self.assertAlmostEqual(p.get_precision(), 0., delta=1e-5)
        p.set_precision(2.0)
        self.assertAlmostEqual(p.get_precision(), 2.0, delta=1e-5)
        self.assertEqual(p.get_density(), "")
        p.set_density("testfile")
        self.assertFilenameEqual(p.get_density(), "testfile")

        p2 = IMP.core.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10,
                4.0, "testfile2")
        self.assertAlmostEqual(p2.get_precision(), 4.0, delta=1e-5)
        self.assertFilenameEqual(p2.get_density(), "testfile2")

    def test_provenanced(self):
        """Test Provenanced decorator"""
        m = IMP.Model()
        p = IMP.core.Provenance.setup_particle(m, IMP.Particle(m))
        pd = IMP.core.Provenanced.setup_particle(m, IMP.Particle(m), p)
        self.assertEqual(pd.get_provenance(), p)
        pd.set_provenance(p)
        self.assertEqual(pd.get_provenance(), p)
        self.assertTrue(IMP.core.Provenanced.get_is_setup(pd))
        self.assertTrue(IMP.core.Provenance.get_is_setup(p))
        self.assertFalse(IMP.core.Provenanced.get_is_setup(p))
        self.assertFalse(IMP.core.Provenance.get_is_setup(pd))

    def test_add_provenance(self):
        """Test add_provenance()"""
        m = IMP.Model()
        prov1 = IMP.core.Provenance.setup_particle(m, IMP.Particle(m))
        prov2 = IMP.core.Provenance.setup_particle(m, IMP.Particle(m))

        p = IMP.Particle(m)
        self.assertFalse(IMP.core.Provenanced.get_is_setup(p))

        IMP.core.add_provenance(m, p, prov1)
        self.assertTrue(IMP.core.Provenanced.get_is_setup(p))
        pd = IMP.core.Provenanced(p)
        self.assertTrue(pd.get_provenance(), prov1)

        IMP.core.add_provenance(m, p, prov2)
        self.assertTrue(IMP.core.Provenanced.get_is_setup(p))
        self.assertTrue(pd.get_provenance(), prov2)
        self.assertTrue(pd.get_provenance().get_previous(), prov1)

    def add_provenance(self, m):
        software = IMP.core.SoftwareProvenance.setup_particle(
                            m, IMP.Particle(m), "IMP", "1.0", "test.org")

        script = IMP.core.ScriptProvenance.setup_particle(
                            m, IMP.Particle(m), "testscript")
        script.set_previous(software)

        struc = IMP.core.StructureProvenance.setup_particle(
                            m, IMP.Particle(m), "testfile", "testchain")
        struc.set_previous(script)

        samp = IMP.core.SampleProvenance.setup_particle(
                            m, IMP.Particle(m), "Monte Carlo", 100, 42)
        samp.set_previous(struc)

        comb = IMP.core.CombineProvenance.setup_particle(
                            m, IMP.Particle(m), 4, 27)
        comb.set_previous(samp)

        filt = IMP.core.FilterProvenance.setup_particle(
                            m, IMP.Particle(m), "Total score", 100.5, 39)
        filt.set_previous(comb)

        clus = IMP.core.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10)
        clus.set_previous(filt)
        return clus

    def check_provenance(self, prov):
        m = prov.get_model()
        self.assertTrue(IMP.core.ClusterProvenance.get_is_setup(m, prov))
        clus = IMP.core.ClusterProvenance(m, prov)
        self.assertEqual(clus.get_number_of_members(), 10)

        prov = prov.get_previous()
        self.assertTrue(IMP.core.FilterProvenance.get_is_setup(m, prov))
        filt = IMP.core.FilterProvenance(m, prov)
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

        prov = prov.get_previous()
        self.assertTrue(IMP.core.ScriptProvenance.get_is_setup(m, prov))
        script = IMP.core.ScriptProvenance(m, prov)
        if sys.platform == 'win32':
            self.assertEqual(script.get_filename(), "testscript")
        else:
            self.assertEqual(script.get_filename(),
                             os.path.abspath("testscript"))

        prov = prov.get_previous()
        self.assertTrue(IMP.core.SoftwareProvenance.get_is_setup(m, prov))
        software = IMP.core.SoftwareProvenance(m, prov)
        self.assertEqual(software.get_software_name(), "IMP")
        self.assertEqual(software.get_version(), "1.0")
        self.assertEqual(software.get_location(), "test.org")

        # Should be no more provenance
        prov = prov.get_previous()
        self.assertFalse(prov)

    def test_clone(self):
        """Test create_clone"""
        m = IMP.Model()
        prov = self.add_provenance(m)
        self.check_provenance(prov)
        self.assertEqual(len(m.get_particle_indexes()), 7)
        newprov = IMP.core.create_clone(prov)
        self.assertEqual(len(m.get_particle_indexes()), 14)
        self.check_provenance(newprov)

    def test_get_all_provenance(self):
        """Test get_all_provenance()"""
        m = IMP.Model()
        p = IMP.Particle(m)
        self.assertEqual(list(IMP.core.get_all_provenance(p)), [])

        prov = self.add_provenance(m)
        IMP.core.Provenanced.setup_particle(p, prov)
        allp = list(IMP.core.get_all_provenance(p))
        self.assertEqual([type(x) for x in allp],
                         [IMP.core.ClusterProvenance, IMP.core.FilterProvenance,
                          IMP.core.CombineProvenance, IMP.core.SampleProvenance,
                          IMP.core.StructureProvenance,
                          IMP.core.ScriptProvenance,
                          IMP.core.SoftwareProvenance])

        allp = list(IMP.core.get_all_provenance(p,
                           types=[IMP.core.ClusterProvenance,
                                  IMP.core.StructureProvenance]))
        self.assertEqual([type(x) for x in allp],
                         [IMP.core.ClusterProvenance,
                          IMP.core.StructureProvenance])

    def test_add_script_provenance(self):
        """Test add_script_provenance()"""
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.core.add_script_provenance(p)
        IMP.core.add_script_provenance(p) # should be a noop
        allp = list(IMP.core.get_all_provenance(p))
        script, = allp
        self.assertIsInstance(script, IMP.core.ScriptProvenance)

    def test_add_software_provenance(self):
        """Test add_software_provenance()"""
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.core.add_software_provenance(p, "my prog", "1.0", "myprog.org")
        IMP.core.add_software_provenance(p, "my prog",
                                         "1.0", "myprog.org") # should be noop
        IMP.core.add_software_provenance(p, "otherprog", "2.0", "othprog.org")
        IMP.core.add_imp_provenance(p)
        IMP.core.add_imp_provenance(p) # should be a noop
        allp = list(IMP.core.get_all_provenance(p))
        p1, p2, p3 = allp
        self.assertEqual(p1.get_software_name(),
                         "Integrative Modeling Platform (IMP)")
        self.assertEqual(p2.get_software_name(), "otherprog")
        self.assertEqual(p3.get_software_name(), "my prog")


if __name__ == '__main__':
    IMP.test.main()
