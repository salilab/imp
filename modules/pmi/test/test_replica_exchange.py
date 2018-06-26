import IMP
import IMP.test
import IMP.pmi.macros

class MockSampler(object):
    def get_number_of_replicas(self):
        return 3

class MockREX(IMP.pmi.macros.ReplicaExchange0):
    # Override the slow init from the base class
    def __init__(self, model):
        self.model = model
        self.pmi2 = False
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
        r.is_multi_state = False
        r.root_hier = h
        r._add_provenance(sampler_md=None, sampler_mc=None)
        # No provenance should have been added, since no sampling is done
        self.assertFalse(IMP.core.Provenanced.get_is_setup(h))

    def test_add_provenance_md(self):
        """Test adding replica exchange provenance, single state MD"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.is_multi_state = False
        r.root_hier = h
        r._add_provenance(sampler_md=True, sampler_mc=None)
        self._check_provenance(r, h, "Molecular Dynamics", iterations=24)

    def test_add_provenance_mc(self):
        """Test adding replica exchange provenance, single state MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.is_multi_state = False
        r.root_hier = h
        r._add_provenance(sampler_md=None, sampler_mc=True)
        self._check_provenance(r, h, "Monte Carlo", iterations=46)

    def test_add_provenance_md_mc(self):
        """Test adding replica exchange provenance, single state MD/MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.is_multi_state = False
        r.root_hier = h
        r._add_provenance(sampler_md=True, sampler_mc=True)
        self._check_provenance(r, h, "Hybrid MD/MC", iterations=70)

    def test_add_provenance_multi_state(self):
        """Test adding replica exchange provenance, multi state MD"""
        m = IMP.Model()
        h1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.is_multi_state = True
        r.root_hiers = [h1, h2]
        r._add_provenance(sampler_md=True, sampler_mc=None)
        for h in (h1, h2):
            self._check_provenance(r, h, "Molecular Dynamics", iterations=24)

    def _check_provenance(self, rex, hier, method, iterations):
        self.assertTrue(IMP.core.Provenanced.get_is_setup(hier))
        prov = IMP.core.Provenanced(hier).get_provenance()
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(prov))
        prov = IMP.core.SampleProvenance(prov)
        self.assertEqual(prov.get_method(), method)
        self.assertEqual(prov.get_number_of_frames(), 100)
        self.assertEqual(prov.get_number_of_iterations(), iterations)
        self.assertEqual(prov.get_number_of_replicas(), 3)

if __name__ == '__main__':
    IMP.test.main()
