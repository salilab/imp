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
        m.evaluate(False)
        sf.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
