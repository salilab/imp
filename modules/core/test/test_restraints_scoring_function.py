import IMP
import IMP.test
import IMP.core
import pickle
try:
    import jax
except ImportError:
    jax = None


idkey = IMP.IntKey("id")


class TestMovedRestraint(IMP.Restraint):
    def __init__(self, m, ps, value, name="TestMovedRestraint %1%"):
        super().__init__(m, name)
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


class TestJAXKeyRestraint(IMP.Restraint):
    def __init__(self, m, ps, name="TestJAXKeyRestraint %1%"):
        super().__init__(m, name)
        self.ps = ps

    def _get_jax(self):
        def jax_restraint(jm):
            return 1.0
        return self._wrap_jax(jax_restraint, keys=[idkey])

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
        r = IMP._ConstRestraint(m, [p], 1)
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
        self.assertEqual(sf.restraints, [r1, r2])
        self.assertNotEqual(sf.restraints, (r1, r2))
        self.assertNotEqual(sf.restraints, [r1])
        othersf = IMP.core.RestraintsScoringFunction([r1, r2])
        self.assertEqual(sf.restraints, othersf.restraints)
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

    def test_pickle(self):
        """Test (un-)pickle of RestraintsScoringFunction"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r = IMP._ConstRestraint(m, [p], 42)
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        sf.set_name("bar")
        self.assertEqual(sf.evaluate(False), 42)

        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        self.assertEqual(newsf.get_name(), "bar")
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")
        self.assertEqual(newsf.evaluate(False), 42)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_keys(self):
        """Test JAX keys returned by RestraintsScoringFunction"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r = TestJAXKeyRestraint(m, p)
        sf = IMP.core.RestraintsScoringFunction([r])
        # TestJAXKeyRestraint should request the 'id' attribute
        ji = sf._get_jax()
        X = ji.get_jax_model()
        self.assertEqual(sorted(X.keys()), ['id', 'r', 'xyz'])

        r = IMP._ConstRestraint(m, [p], 42)
        sf = IMP.core.RestraintsScoringFunction([r])
        # ConstRestraint doesn't request any keys; we should get the default
        ji = sf._get_jax()
        X = ji.get_jax_model()
        self.assertEqual(sorted(X.keys()), ['r', 'xyz'])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_score(self):
        """Test JAX RestraintsScoringFunction score"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r1 = IMP._ConstRestraint(m, [p], 42)
        r2 = IMP._ConstRestraint(m, [p], 18)
        sf = IMP.core.RestraintsScoringFunction([r1, r2])
        ji = sf._get_jax()
        X = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(X), 60.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
