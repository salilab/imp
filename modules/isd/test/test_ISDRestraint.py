#!/usr/bin/env python

# imp general
import IMP
import IMP.core
import IMP.isd

# unit testing framework
import IMP.test

class ISDRestraint(IMP.isd.ISDRestraint):
    def __init__(self,m):
        IMP.isd.ISDRestraint.__init__(self, m, "ISDRestraint %1%")



class TestISDRestraint(IMP.test.TestCase):

    def test_setup(self):
        m=IMP.Model()
        isdr=ISDRestraint(m)
        self.assertTrue(hasattr(isdr, 'get_probability'))
        self.assertTrue(hasattr(isdr, 'unprotected_evaluate'))
        self.assertTrue(hasattr(isdr, 'do_get_inputs'))
        self.assertTrue(isdr.unprotected_evaluate(None) == 0.0)

    def test_restraintset(self):
        m=IMP.Model()
        rs=IMP.RestraintSet(m)
        rs.add_restraint(ISDRestraint(m))
        for r in rs.get_restraints():
            isdr=IMP.isd.ISDRestraint.get_from(r)
            self.assertTrue(hasattr(isdr, 'get_probability'))

if __name__ == '__main__':
    IMP.test.main()
