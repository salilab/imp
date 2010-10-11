import IMP
import IMP.test
import IMP.misc
import IMP.core

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""

    def _random_hierarchy(self, m, depth=2, width=4):
        p= IMP.Particle(m)
        h = IMP.core.Hierarchy.setup_particle(p)
        if depth== 0:
            d = IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()),
                                              .1))
        else:
            children= []
            for x in range(0, width):
                children.append(self._random_hierarchy(m, depth-1))
            #cps= [x.get_particle() for x in children]
            d= IMP.core.XYZR.setup_particle(p)
            #print children
            IMP.core.set_enclosing_sphere(d, children)
            for c in children:
                h.add_child(c)
        return h

    def test_rops(self):
        """Checking close pairs pair score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        threshold = 5.0
        r0= self._random_hierarchy(m)
        r1= self._random_hierarchy(m)
        ls0= IMP.core.get_leaves(r0)
        ls1= IMP.core.get_leaves(r1)
        print ls0
        print ls1
        cpr= IMP.core.LeavesRefiner(r0.get_traits())
        lps= IMP.misc.LogPairScore()
        cpps= IMP.core.ClosePairsPairScore(lps, cpr, threshold)
        cpps.evaluate(IMP.ParticlePair(r0.get_particle(), r1.get_particle()), None)
        print str(len(lps.get_particle_pairs())) +" pairs", "in", threshold
        for pp in lps.get_particle_pairs():
            print pp[0].get_name(), pp[1].get_name()
        for l0 in ls0:
            for l1 in ls1:
                d0= IMP.core.XYZ.decorate_particle(l0.get_particle())
                d1= IMP.core.XYZ.decorate_particle(l1.get_particle())
                if (IMP.core.get_distance(d0, d1) < .95*threshold):
                    print "looking for", l0.get_particle().get_name() + " " \
                        + l1.get_particle().get_name(), IMP.core.get_distance(d0, d1)
                    self.assertTrue(lps.get_contains(IMP.ParticlePair(l0.get_particle(),
                                                                   l1.get_particle())))
                else:
                    self.assertFalse(lps.get_contains(IMP.ParticlePair(l0.get_particle(),
                                                                       l1.get_particle())))




if __name__ == '__main__':
    IMP.test.main()
