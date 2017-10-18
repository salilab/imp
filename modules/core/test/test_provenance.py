from __future__ import print_function
import IMP
import IMP.test
import IMP.core

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

    def test_structure_provenance(self):
        """Test StructureProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.StructureProvenance.setup_particle(m, IMP.Particle(m),
                               "testfile", "testchain")
        self.assertTrue(IMP.core.StructureProvenance.get_is_setup(p))
        self.assertEqual(p.get_filename(), "testfile")
        self.assertEqual(p.get_chain_id(), "testchain")

    def test_sample_provenance(self):
        """Test SampleProvenance decorator"""
        m = IMP.Model()
        p = IMP.core.SampleProvenance.setup_particle(m, IMP.Particle(m),
                               "Monte Carlo", 100)
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(p))
        self.assertEqual(p.get_method(), "Monte Carlo")
        p.set_method("Molecular Dynamics")
        self.assertEqual(p.get_method(), "Molecular Dynamics")
        if IMP.get_check_level() == IMP.USAGE_AND_INTERNAL:
            self.assertRaises(IMP.UsageError, p.set_method, "Garbage")
        self.assertEqual(p.get_number_of_frames(), 100)
        p.set_number_of_frames(200)
        self.assertEqual(p.get_number_of_frames(), 200)
        self.assertEqual(p.get_number_of_iterations(), 1)
        p.set_number_of_iterations(42)
        self.assertEqual(p.get_number_of_iterations(), 42)

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
        p = IMP.core.FilterProvenance.setup_particle(m, IMP.Particle(m), 100.5,
                                                     42)
        self.assertTrue(IMP.core.FilterProvenance.get_is_setup(p))
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

if __name__ == '__main__':
    IMP.test.main()
