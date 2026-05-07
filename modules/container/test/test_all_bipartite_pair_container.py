import IMP
import IMP.test
import IMP.container
import IMP.core
import itertools
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def _setup_system(self):
        m = IMP.Model()
        ps0 = IMP.get_indexes(self.create_particles_in_box(m, 10))
        ps1 = IMP.get_indexes(self.create_particles_in_box(m, 10))
        for p in itertools.chain(ps0, ps1):
            d = IMP.core.XYZR.setup_particle(m, p, 1.0)

        pc0 = IMP.container.ListSingletonContainer(m, ps0)
        pc1 = IMP.container.ListSingletonContainer(m, ps1)
        apss = IMP.container.AllBipartitePairContainer(pc0, pc1)
        return m, apss

    def test_indexes(self):
        """Test indexes of AllBipartitePairContainer"""
        m, apss = self._setup_system()
        # Each of the 10 ps0 particles should interact with each of the
        # 10 ps1 particles
        self.assertEqual(len(apss.get_indexes()), 100)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation"""
        m, apss = self._setup_system()
        ps = IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(4.0, 4.0))
        r = IMP.container.PairsRestraint(ps, apss)

        imp_score = r.evaluate(False)
        jax_score = r._evaluate_jax()
        self.assertAlmostEqual(imp_score, jax_score, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
