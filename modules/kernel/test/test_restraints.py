import IMP
import IMP.core
import IMP.test
try:
    import jax
except ImportError:
    jax = None


class PythonRestraint(IMP.Restraint):
    """Do-nothing restraint implemented in Python"""
    def __init__(self, m):
        super().__init__(m, "PythonRestraint %1%")


class Tests(IMP.test.TestCase):

    """Test RestraintSets"""

    def test_printing(self):
        """Test ref counting works with restraints and scoring functions"""
        m = IMP.Model()
        # make sure that sf keeps the restraint alive
        sf = IMP._ConstRestraint(m, [], 1).create_scoring_function()
        IMP.set_log_level(IMP.MEMORY)
        # trigger cleanup
        m.update()
        sf.evaluate(False)

    def test_aggregate(self):
        """Test get_is_aggregate()"""
        m = IMP.Model()
        cr = IMP._ConstRestraint(m, [], 1)
        self.assertFalse(cr.get_is_aggregate())

    def test_object_info(self):
        """Test restraint object get_type_name() and get_version_info()"""
        m = IMP.Model()
        cr = IMP._ConstRestraint(m, [], 1)
        self.assertEqual(cr.get_type_name(), "_ConstRestraint")
        self.assertEqual(cr.get_version_info().get_module(), "IMP")
        # Should get the same information using the Restraint base class
        r_cr = IMP.Restraint.get_from(cr)
        self.assertIs(type(r_cr), IMP.Restraint)
        self.assertEqual(r_cr.get_type_name(), "_ConstRestraint")
        self.assertEqual(r_cr.get_version_info().get_module(), "IMP")
        # Should get the same information using the Object base class
        mk = IMP.ModelKey("data_key")
        m.add_data(mk, cr)
        obj_cr = m.get_data(mk)
        self.assertIs(type(obj_cr), IMP.Object)
        self.assertEqual(obj_cr.get_type_name(), "_ConstRestraint")
        self.assertEqual(obj_cr.get_version_info().get_module(), "IMP")
        self.assertIs(type(obj_cr.get_derived_object()), IMP._ConstRestraint)

    def test_director_object_info(self):
        """Test get_derived_object() on SWIG director objects"""
        m = IMP.Model()
        cr = PythonRestraint(m)
        # Cast to IMP.Restraint base class
        sf = IMP.core.RestraintsScoringFunction([cr])
        r_cr = sf.restraints[0]
        self.assertIs(type(r_cr), IMP.Restraint)
        # get_derived_object() should be able to recover the original object
        new_cr = r_cr.get_derived_object()
        self.assertIs(type(new_cr), PythonRestraint)
        self.assertIs(new_cr, cr)

    def test_get_jax(self):
        """Test get_jax() on Restraint base class"""
        m = IMP.Model()
        cr = PythonRestraint(m)
        self.assertRaises(NotImplementedError, cr._get_jax)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_const_singleton_score(self):
        """Test JAX implementation of _ConstSingletonScore"""
        m = IMP.Model()
        p = IMP.Particle(m)
        ss = IMP._ConstSingletonScore(10.0)
        r = IMP.core.SingletonRestraint(m, ss, p)
        ji = r._get_jax()
        jm = ji.get_jax_model()
        s = jax.jit(ji.score_func)
        self.assertAlmostEqual(s(jm), 10.0, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_const_pair_score(self):
        """Test JAX implementation of _ConstPairScore"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        ps = IMP._ConstPairScore(10.0)
        r = IMP.core.PairRestraint(m, ps, (p1, p2))
        ji = r._get_jax()
        jm = ji.get_jax_model()
        s = jax.jit(ji.score_func)
        self.assertAlmostEqual(s(jm), 10.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
