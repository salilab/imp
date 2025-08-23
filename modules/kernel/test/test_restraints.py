import IMP
import IMP.test


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


if __name__ == '__main__':
    IMP.test.main()
