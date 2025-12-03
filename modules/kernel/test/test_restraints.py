import IMP
import IMP.core
import IMP.test


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


if __name__ == '__main__':
    IMP.test.main()
