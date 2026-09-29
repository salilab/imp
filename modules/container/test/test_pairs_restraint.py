import IMP
import IMP.test
import IMP.container


class Tests(IMP.test.TestCase):

    """Test PairsRestraint"""

    def test_accessors(self):
        """Test accessors of PairsRestraint"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        lpc = IMP.container.ListPairContainer(m, [(p1, p2)])
        p1 = IMP._ConstPairScore(10.0)
        r1 = IMP.container.PairsRestraint(p1, lpc)

        s = r1.get_score_object()
        self.assertIsInstance(s, IMP.PairScore)
        c = r1.get_container()
        self.assertIsInstance(c, IMP.PairContainer)

    def test_clear_moved_cache(self):
        """Test PairsRestraint.clear_moved_cache()"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        d1 = IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(0,0,0))
        p2 = IMP.Particle(m)
        d2 = IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(5,0,0))
        lpc = IMP.container.ListPairContainer(m, [(p1, p2)])
        s1 = IMP.core.DistancePairScore(IMP.core.Linear(0.0, 1.0))
        r1 = IMP.container.PairsRestraint(s1, lpc)
        sf = IMP.core.RestraintsScoringFunction([r1])

        # Should return the d1-d2 distance, i.e. 5.0
        self.assertAlmostEqual(sf.evaluate_moved(False, [], []), 5.0,
                               delta=1e-6)

        # If we move a particle but don't report it (e.g. start of
        # MC optimize() with reset_pis from the end of a previous optimize(),
        # that will be lost) we should get the old (wrong) score; this will
        # be caught by an internal check in debug mode
        d2.set_coordinates(IMP.algebra.Vector3D(7,0,0))
        if IMP.get_check_level() >= IMP.USAGE_AND_INTERNAL:
            self.assertRaises(IMP.InternalException, sf.evaluate_moved,
                              False, [], [])
        else:
            self.assertAlmostEqual(sf.evaluate_moved(False, [], []), 5.0,
                                   delta=1e-6)

        # Reset of caches should force a full evaluation
        sf.clear_moved_cache()
        self.assertAlmostEqual(sf.evaluate_moved(False, [], []), 7.0,
                               delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
