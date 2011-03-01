import sys
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.restrainer
import time
import random


class DOMINOTests(IMP.test.TestCase):
    def test_global_min2(self):
        """Testing ordering"""
        m= IMP.Model()
        ps=[]
        pst= IMP.domino.ParticleStatesTable()
        state= IMP.domino.IndexStates(10);
        for i in range(0,10):
            ps.append(IMP.Particle(m));
            pst.set_particle_states(ps[-1], state)
        eqft= IMP.domino.EquivalenceSubsetFilterTable(pst)
        ps.sort()
        order= IMP.domino.get_order(IMP.domino.Subset(ps),
                                    [eqft]);
        print "order is", order
        for i,e in enumerate(order):
            self.assertEqual(i,9-e)


if __name__ == '__main__':
    IMP.test.main()
