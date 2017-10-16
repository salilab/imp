from __future__ import print_function
import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):

    def test_provenance(self):
        """Test Provenance decorator"""
        m = IMP.Model()
        p = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        self.assertTrue(IMP.atom.Provenance.get_is_setup(p))
        self.assertFalse(p.get_previous_state())

        p2 = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        p.set_previous_state(p2)
        self.assertEqual(p.get_previous_state(), p2)

    def test_structure_provenance(self):
        """Test StructureProvenance decorator"""
        m = IMP.Model()
        p = IMP.atom.StructureProvenance.setup_particle(m, IMP.Particle(m),
                               "testfile", "testchain")
        self.assertTrue(IMP.atom.StructureProvenance.get_is_setup(p))
        self.assertEqual(p.get_filename(), "testfile")
        self.assertEqual(p.get_chain_id(), "testchain")

    def test_sample_provenance(self):
        """Test SampleProvenance decorator"""
        m = IMP.Model()
        p = IMP.atom.SampleProvenance.setup_particle(m, IMP.Particle(m),
                               "Monte Carlo", 100)
        self.assertTrue(IMP.atom.SampleProvenance.get_is_setup(p))
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
        p = IMP.atom.CombineProvenance.setup_particle(m, IMP.Particle(m), 5, 42)
        self.assertTrue(IMP.atom.CombineProvenance.get_is_setup(p))
        self.assertEqual(p.get_number_of_runs(), 5)
        p.set_number_of_runs(7)
        self.assertEqual(p.get_number_of_runs(), 7)
        self.assertEqual(p.get_number_of_frames(), 42)
        p.set_number_of_frames(100)
        self.assertEqual(p.get_number_of_frames(), 100)

    def test_cluster_provenance(self):
        """Test ClusterProvenance decorator"""
        m = IMP.Model()
        p = IMP.atom.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10)
        self.assertTrue(IMP.atom.ClusterProvenance.get_is_setup(p))
        self.assertEqual(p.get_number_of_members(), 10)
        p.set_number_of_members(42)
        self.assertEqual(p.get_number_of_members(), 42)

    def test_provenanced(self):
        """Test Provenanced decorator"""
        m = IMP.Model()
        p = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        pd = IMP.atom.Provenanced.setup_particle(m, IMP.Particle(m), p)
        self.assertEqual(pd.get_provenance(), p)
        pd.set_provenance(p)
        self.assertTrue(IMP.atom.Provenanced.get_is_setup(pd))
        self.assertFalse(IMP.atom.Provenanced.get_is_setup(p))
        self.assertFalse(IMP.atom.Provenance.get_is_setup(pd))

    def test_add_provenance(self):
        """Test add_provenance()"""
        m = IMP.Model()
        prov1 = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        prov2 = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))

        p = IMP.Particle(m)
        self.assertFalse(IMP.atom.Provenanced.get_is_setup(p))

        IMP.atom.add_provenance(m, p, prov1)
        self.assertTrue(IMP.atom.Provenanced.get_is_setup(p))
        pd = IMP.atom.Provenanced(p)
        self.assertTrue(pd.get_provenance(), prov1)

        IMP.atom.add_provenance(m, p, prov2)
        self.assertTrue(IMP.atom.Provenanced.get_is_setup(p))
        self.assertTrue(pd.get_provenance(), prov2)
        self.assertTrue(pd.get_provenance().get_previous_state(), prov1)

if __name__ == '__main__':
    IMP.test.main()
