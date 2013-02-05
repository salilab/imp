import IMP
import IMP.test
import IMP.container
import math


class Tests(IMP.test.TestCase):
    """Tests for all pairs pair container"""

    def test_allp2(self):
        """Check the event optimizer states"""
        m= IMP.Model()
        ps= []
        for i in range(0,5):
            p= IMP.Particle(m)
            ps.append(p)
        ap= IMP.container.ListSingletonContainer(ps)
        cp= IMP.core.ConstantSingletonPredicate(1)
        os= IMP.container.EventSingletonsOptimizerState(cp, ap,
                                                        1, 1, 10)
        opt= IMP.core.MonteCarlo(m)
        opt.add_optimizer_state(os)
        self.assertRaises(IMP.base.EventException, os.update)



if __name__ == '__main__':
    IMP.test.main()
