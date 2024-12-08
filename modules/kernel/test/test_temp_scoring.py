import IMP
import IMP.test
import IMP.core
import io
import random


class DummyRestraint(IMP.Restraint):

    """Dummy do-nothing restraint"""

    def __init__(self, m, ps=[], cs=[]):
        IMP.Restraint.__init__(self, m, "DummyRestraint %1%")
        self.ps = ps
        self.cs = cs

    def unprotected_evaluate(self, accum):
        return 0.

    def get_version_info(self):
        return IMP.get_module_version_info()

    def do_get_inputs(self):
        return self.ps + self.cs


class Tests(IMP.test.TestCase):

    def test_temp_restraints(self):
        """Check that scoring functions are cleaned up"""
        dirchk = IMP.test.RefCountChecker(self)
        IMP.set_log_level(IMP.MEMORY)
        m = IMP.Model("M")
        #self.assertRaises(IndexError, m.get_restraint, 0);
        r = DummyRestraint(m)
        r.set_name("dummy")
        dirchk.assert_number(2)
        print(r.evaluate(False))
        dirchk.assert_number(3)
        del r
        dirchk.assert_number(1)
        del m
        dirchk.assert_number(0)

if __name__ == '__main__':
    IMP.test.main()
