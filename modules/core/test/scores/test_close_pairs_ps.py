import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.core

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""

    def _random_hierarchy(self, m, depth=2, width=4):
        p= IMP.Particle(m)
        h = IMP.core.HierarchyDecorator.create(p)
        if depth== 0:
            d = IMP.core.XYZRDecorator.create(p, IMP.algebra.Sphere3D(IMP.algebra.random_vector_in_unit_box(),
                                              .1))
        else:
            children= [self._random_hierarchy(m, depth-1) for x in range(0,width)]
            cps= [x.get_particle() for x in children]
            d= IMP.core.XYZRDecorator.create(p)
            IMP.core.set_enclosing_sphere(d, cps)
            for c in children:
                h.add_child(c)
        return h

    def test_rops(self):
        """Checking close pairs pair score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        threshold = 0
        r0= self._random_hierarchy(m)
        r1= self._random_hierarchy(m)
        ls0= IMP.core.get_leaves(r0)
        ls1= IMP.core.get_leaves(r1)
        cpr= IMP.core.ChildrenRefiner(r0.get_traits())
        lps= IMP.misc.LogPairScore()
        cpps= IMP.core.ClosePairsPairScore(cpr, lps, threshold)
        cpps.evaluate(r0.get_particle(), r1.get_particle(), None)
        print str(len(lps.get_particle_pairs())) +" pairs"
        for pp in lps.get_particle_pairs():
            pp.show()
            print
        for l0 in ls0:
            for l1 in ls1:
                d0= IMP.core.XYZRDecorator.cast(l0)
                d1= IMP.core.XYZRDecorator.cast(l1)
                if (IMP.core.distance(d0, d1) < threshold):
                    print l0.get_name() + " " + l1.get_name()
                    self.assert_(lps.get_contains(IMP.ParticlePair(l0, l1)))
                else:
                    self.assert_(not lps.get_contains(IMP.ParticlePair(l0, l1)))




if __name__ == '__main__':
    unittest.main()
