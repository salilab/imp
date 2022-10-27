import IMP
import IMP.test
import IMP.pmi.macros

class MockSampler(object):
    def get_number_of_replicas(self):
        return 3

class MockREX(IMP.pmi.macros.ReplicaExchange):
    # Override the slow init from the base class
    def __init__(self, model):
        self.model = model
        self.vars = {'molecular_dynamics_steps':12,
                     'monte_carlo_steps':23,
                     'num_sample_rounds':2,
                     'number_of_frames':100}
        self.replica_exchange_object = MockSampler()

class Tests(IMP.test.TestCase):

    def test_add_provenance_none(self):
        """Test adding replica exchange provenance, no MD or MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=None, sampler_mc=None)
        # No provenance should have been added, since no sampling is done
        self.assertFalse(IMP.core.Provenanced.get_is_setup(h))

    def test_add_provenance_md(self):
        """Test adding replica exchange provenance, MD"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=True, sampler_mc=None)
        self._check_provenance(r, h, "Molecular Dynamics", iterations=24)

    def test_add_provenance_mc(self):
        """Test adding replica exchange provenance, MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=None, sampler_mc=True)
        self._check_provenance(r, h, "Monte Carlo", iterations=46)

    def test_add_provenance_md_mc(self):
        """Test adding replica exchange provenance, MD/MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=True, sampler_mc=True)
        self._check_provenance(r, h, "Hybrid MD/MC", iterations=70)

    def _check_provenance(self, rex, hier, method, iterations):
        self.assertTrue(IMP.core.Provenanced.get_is_setup(hier))
        prov = IMP.core.Provenanced(hier).get_provenance()
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(prov))
        prov = IMP.core.SampleProvenance(prov)
        self.assertEqual(prov.get_method(), method)
        self.assertEqual(prov.get_number_of_frames(), 100)
        self.assertEqual(prov.get_number_of_iterations(), iterations)
        self.assertEqual(prov.get_number_of_replicas(), 3)

    def test_test_save_coordinates_mode(self):
        """Test ReplicaExchange test_mode with save_coordinates_mode"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_flexible_beads(nup84)
        rex = IMP.pmi.macros.ReplicaExchange(
            m, root_hier=hier,
            monte_carlo_sample_objects=dof.get_movers(), number_of_frames=2,
            test_mode=True, save_coordinates_mode="25th_score")
        rex.execute_macro()


if __name__ == '__main__':
    IMP.test.main()
