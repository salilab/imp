import IMP
import IMP.test
import IMP.core

rk = IMP.FloatKey("radius")

class Tests(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Checking refine pairs pair score"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        pp= IMP.Particle(m)
        hpp= IMP.core.Hierarchy.setup_particle(pp)
        c=[]
        for i in range(0,10):
            p= IMP.Particle(m)
            hp= IMP.core.Hierarchy.setup_particle(p)
            hpp.add_child(hp)
            c.append(p)
        pr= IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())
        cps= IMP.kernel._ConstPairScore(1)
        rps= IMP.core.RefinedPairsPairScore(pr, cps)
        ppp=(pp, pp)
        print type(rps)
        print type(rps.evaluate)
        da=IMP.DerivativeAccumulator()
        print type(pp)
        print type(ppp)
        self.assertEqual(rps.evaluate(ppp, da), 100)





if __name__ == '__main__':
    IMP.test.main()
