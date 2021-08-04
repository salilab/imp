from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.container

class Tests(IMP.test.TestCase):

    def test_no_model(self):
        """Check that printing particle indexes in python works"""
        pi = IMP.ParticleIndex(1)
        strpi = str(pi)
        print(strpi)
        self.assertEqual(strpi, "1")

    def test_indexes(self):
        """Check that ParticleIndexesAdaptor works"""
        pis = [IMP.ParticleIndex(i) for i in range(0, 10)]
        opis = IMP._take_particle_indexes_adaptor(pis)
        self.assertNumPyArrayEqual(opis, pis)

    def test_coerce_particle_to_index(self):
        """Check that Particle is coerced to ParticleIndex in Python"""
        m = IMP.Model()
        pi = m.add_particle("P1")
        p = m.get_particle(pi)
        self.assertEqual(m.get_particle(p), p)

    def test_coerce_decorator_to_index(self):
        """Check that Decorator is coerced to ParticleIndex in Python"""
        m = IMP.Model()
        pi = m.add_particle("P1")
        d = IMP.core.XYZ.setup_particle(m, pi)
        p = m.get_particle(pi)
        self.assertEqual(m.get_particle(d), p)

    def test_coerce_tuples_to_index(self):
        """Check that tuples are coerced to ParticleIndexPair in Python"""
        m = IMP.Model()
        pi1 = m.add_particle("P1")
        pi2 = m.add_particle("P2")
        p1 = m.get_particle(pi1)
        p2 = m.get_particle(pi2)
        d1 = IMP.core.XYZ.setup_particle(m, pi1)
        d2 = IMP.core.XYZ.setup_particle(m, pi2)
        ps = IMP.core.DistancePairScore(IMP.core.Harmonic(1,1))
        r = IMP.core.PairRestraint(m, ps, (pi1, pi2))
        r.set_was_used(True)
        r = IMP.core.PairRestraint(m, ps, (p1, p2))
        r.set_was_used(True)
        r = IMP.core.PairRestraint(m, ps, (d1, d2))
        r.set_was_used(True)

    def test_coerce_tuples_list_to_index(self):
        """Check that list is coerced to ParticleIndexPairs in Python"""
        m = IMP.Model()
        pi1 = m.add_particle("P1")
        pi2 = m.add_particle("P2")
        p1 = m.get_particle(pi1)
        p2 = m.get_particle(pi2)
        d1 = IMP.core.XYZ.setup_particle(m, pi1)
        d2 = IMP.core.XYZ.setup_particle(m, pi2)
        c = IMP.container.ListPairContainer(m, [(pi1, pi2)])
        c.set_was_used(True)
        self.assertNumPyArrayEqual(c.get_indexes(), [(pi1,pi2)])
        c = IMP.container.ListPairContainer(m, [(p1, p2)])
        c.set_was_used(True)
        self.assertNumPyArrayEqual(c.get_indexes(), [(pi1,pi2)])
        c = IMP.container.ListPairContainer(m, [(d1, d2)])
        c.set_was_used(True)
        self.assertNumPyArrayEqual(c.get_indexes(), [(pi1,pi2)])


if __name__ == '__main__':
    IMP.test.main()
