from __future__ import print_function
import IMP
import IMP.test
import IMP.container
import math


class Tests(IMP.test.TestCase):

    """Tests for all pairs pair container"""

    def test_allp(self):
        """Checking decomposition of plural restraints"""
        m = IMP.Model()
        ps = []
        for i in range(0, 10):
            p = m.add_particle("P%d" % i)
            IMP.core.XYZR.setup_particle(m, p)
            ps.append(p)
        lp = IMP.container.ListSingletonContainer(m, ps)
        apc = IMP.container.AllPairContainer(lp)
        dps = IMP.core.SoftSpherePairScore(1)
        r = IMP.container.PairsRestraint(dps, apc)
        dr = r.create_decomposition()
        print(dr.evaluate(False))
        cdr = IMP.RestraintSet.get_from(dr).get_restraints()
        # IMP.set_log_level(IMP.VERBOSE)
        print("deling")
        del dr
        print("eval single")
        for rc in cdr:
            print(rc.evaluate(False))
if __name__ == '__main__':
    IMP.test.main()
