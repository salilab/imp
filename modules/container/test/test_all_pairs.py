from __future__ import print_function
import IMP
import IMP.test
import IMP.container
import math

class Tests(IMP.test.TestCase):

    """Tests for all pairs pair container"""

    def test_allp(self):
        """Check AllPairContainer"""
        m = IMP.Model()
        ps = [m.add_particle("p%d" % i) for i in range(0, 50)]
        lp = IMP.container.ListSingletonContainer(m, ps)
        apc = IMP.container.AllPairContainer(lp)
        #self.assertEqual(lp.get_ref_count(), 2)
        # use that names are unique
        psl = set()
        IMP.set_log_level(IMP.VERBOSE)
        for pair in apc.get():
            # print ap[-1][0]
            # print ap[-1][1]
            n0 = pair[0].get_name()
            n1 = pair[1].get_name()
            if n0 < n1:
                dp = (n0, n1)
            else:
                dp = (n1, n0)
            print(dp)
            # print psl
            self.assertNotIn(dp, psl, "Pair " + n0 + " and " + n1
                             + " is already in list " + str(psl))
            psl = psl.union([dp])
        print(psl)
        self.assertEqual(len(apc.get()),
                         len(lp.get()) * (len(lp.get()) - 1) / 2)

if __name__ == '__main__':
    IMP.test.main()
