import sys
import unittest
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time
import random


class DOMINOTests(IMP.test.TestCase):
    def _test_global_min2(self):
        """Testing static container"""
        m= IMP.Model()
        ps=[]
        for i in range(0, 20):
            p= IMP.Particle(m)
            IMP.core.XYZ.setup_particle(p)
            ps.append(p)
        lsc= IMP.container.ListSingletonContainer(ps)
        cpc= IMP.container.ClosePairContainer(lsc, 5)
        dg= m.get_dependency_graph()
        self.assert_(IMP.domino2.get_is_static_container(lsc, dg, ps))
        self.assert_(not IMP.domino2.get_is_static_container(cpc, dg, ps))


if __name__ == '__main__':
    unittest.main()
