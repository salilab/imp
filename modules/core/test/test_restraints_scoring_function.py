import IMP
import IMP.test
import IMP.core


class TestMovedRestraint(IMP.Restraint):
    def __init__(self, m, ps, value, name="TestMovedRestraint %1%"):
        IMP.Restraint.__init__(self, m, name)
        self.ps = ps
        self.value = value

    def unprotected_evaluate(self, accum):
        self.moved_pis = None
        self.reset_pis = None
        return self.value

    def unprotected_evaluate_moved(self, accum, moved_pis, reset_pis):
        self.moved_pis = moved_pis
        self.reset_pis = reset_pis
        return self.value * 10.

    def do_get_inputs(self):
        return self.ps


class Tests(IMP.test.TestCase):

    """Test RestraintSets"""

    def _make_stuff(self):
        m = IMP.Model()
        rs = IMP.RestraintSet(m, .5, "RS")
        r0 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r0)
        r1 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r1)
        r2 = IMP._ConstRestraint(m, [], 1)
        return (m, rs, r0, r1, r2)

    def test_weights2(self):
        """Test that sets can be weighted"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        sf1 = IMP.core.RestraintsScoringFunction([rs, r2])
        self.assertEqual(sf1.evaluate(False), 2)
        rs.set_weight(1)
        self.assertEqual(sf1.evaluate(False), 3)

        sf = IMP.core.RestraintsScoringFunction([r0, r1, r2])
        self.assertEqual(sf.evaluate_if_good(False), 3)
        self.assertEqual(rs.evaluate(False), 2)

    def test_weights(self):
        """Test that restraints decompose ok"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r = IMP._ConstRestraint(1, [p])
        rd = r.create_decomposition()
        self.assertEqual(r.evaluate(False), rd.evaluate(False))
        ra = IMP.get_restraints([r])
        rda = IMP.get_restraints([rd])
        sf = IMP.core.RestraintsScoringFunction(ra)
        sfda = IMP.core.RestraintsScoringFunction(rda)
        self.assertEqual(sf.evaluate(False), sfda.evaluate(False))

    def test_scoring(self):
        """Test basic scoring of RestraintsScoringFunction"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r1 = TestMovedRestraint(m, [p], value=42.)
        sf = IMP.core.RestraintsScoringFunction([r1])
        self.assertAlmostEqual(sf.evaluate(False), 42., delta=1e-6)
        self.assertAlmostEqual(sf.evaluate_if_good(False), 42., delta=1e-6)
        self.assertAlmostEqual(sf.evaluate_if_below(False, 1e6),
                               42., delta=1e-6)
        self.assertIsNone(r1.moved_pis)
        self.assertIsNone(r1.reset_pis)

    def test_scoring_moved(self):
        """Test scoring of RestraintsScoringFunction with moved particles"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r1 = TestMovedRestraint(m, [p], value=42.)
        sf = IMP.core.RestraintsScoringFunction([r1])
        self.assertAlmostEqual(sf.evaluate_moved(False, [p], []),
                               420., delta=1e-6)
        self.assertAlmostEqual(sf.evaluate_moved_if_good(False, [p], []),
                               420., delta=1e-6)
        self.assertAlmostEqual(sf.evaluate_moved_if_below(False, [p], [], 1e6),
                               420., delta=1e-6)
        self.assertEqual(r1.moved_pis, IMP.get_indexes([p]))
        self.assertEqual(len(r1.reset_pis), 0)

    def test_python_list(self):
        """Test Python list-like access to restraints"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r1 = TestMovedRestraint(m, [p], value=42.)
        r2 = TestMovedRestraint(m, [p], value=99.)
        sf = IMP.core.RestraintsScoringFunction([r1])
        self.assertAlmostEqual(sf.evaluate(False), 42., delta=1e-6)

        self.assertEqual(len(sf.restraints), 1)
        self.assertIn(r1, sf.restraints)
        self.assertNotIn(r2, sf.restraints)

        del sf.restraints
        self.assertEqual(len(sf.restraints), 0)
        self.assertAlmostEqual(sf.evaluate(False), 0., delta=1e-6)

        sf.restraints = [r1, r2]
        self.assertAlmostEqual(sf.evaluate(False), 141., delta=1e-6)
        sf.restraints.pop()
        self.assertAlmostEqual(sf.evaluate(False), 42., delta=1e-6)
        sf.restraints.pop()
        self.assertAlmostEqual(sf.evaluate(False), 0., delta=1e-6)
        self.assertRaises(IndexError, sf.restraints.pop)

        sf.restraints.append(r1)
        sf.restraints.extend([r2])
        self.assertAlmostEqual(sf.evaluate(False), 141., delta=1e-6)
        self.assertEqual(sf.restraints.index(r1), 0)
        self.assertEqual(sf.restraints.index(r2), 1)
        self.assertRaises(ValueError, sf.restraints.index, r1, start=6)
        self.assertRaises(ValueError, sf.restraints.index, r1, start=0, stop=0)
        self.assertEqual(sf.restraints[0], r1)
        self.assertEqual(sf.restraints[1], r2)
        self.assertRaises(IndexError, lambda: sf.restraints[42])
        self.assertRaises(IndexError, lambda: sf.restraints[-42])
        del sf.restraints[1]
        def _delfunc():
            del sf.restraints[42]
        self.assertRaises(IndexError, _delfunc)
        self.assertRaises(ValueError, sf.restraints.index, r2)


if __name__ == '__main__':
    IMP.test.main()
