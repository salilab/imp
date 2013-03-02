import IMP
import IMP.test
import IMP.core

rk = IMP.FloatKey("radius")

class Tests(IMP.test.TestCase):
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
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        threshold = 5.0
        r0= self._random_hierarchy(m)
        r1= self._random_hierarchy(m)
        ls0= IMP.core.get_leaves(r0)
        ls1= IMP.core.get_leaves(r1)
        print ls0
        print ls1
        cpr= IMP.core.LeavesRefiner(r0.get_traits())
        lps= IMP.kernel._LogPairScore()
        cpps= IMP.core.ClosePairsPairScore(lps, cpr, threshold)
        cpps.evaluate((r0, r1), None)
        print str(len(lps.get_particle_pairs())) +" pairs", "in", threshold
        for pp in lps.get_particle_pairs():
            print pp[0].get_name(), pp[1].get_name()
        for l0 in ls0:
            for l1 in ls1:
                d0= IMP.core.XYZR.decorate_particle(l0.get_particle())
                d1= IMP.core.XYZR.decorate_particle(l1.get_particle())
                if (IMP.core.get_distance(d0, d1) < .95*threshold):
                    print "looking for", l0.get_particle().get_name() + " " \
                        + l1.get_particle().get_name(), IMP.core.get_distance(d0, d1)
                    self.assertTrue(lps.get_contains((l0.get_particle(),
                                                                   l1.get_particle())))
                else:
                    self.assertFalse(lps.get_contains((l0.get_particle(),
                                                                       l1.get_particle())))
    def test_rops2(self):
        """Checking k-close pairs pair score"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        threshold = 5.0
        r0= self._random_hierarchy(m)
        r1= self._random_hierarchy(m)
        ls0= IMP.core.get_leaves(r0)
        ls1= IMP.core.get_leaves(r1)
        print ls0
        print ls1
        cpr= IMP.core.LeavesRefiner(r0.get_traits())
        lps= IMP.kernel._LogPairScore()
        k=1
        cpps= IMP.core.KClosePairsPairScore(lps, cpr, k)
        cpps.evaluate((r0, r1), None)
        print str(len(lps.get_particle_pairs())) +" pairs", "in", k
        for pp in lps.get_particle_pairs():
            print pp[0].get_name(), pp[1].get_name()
        apd= [(IMP.core.get_distance(IMP.core.XYZR(p0),
                                     IMP.core.XYZR(p1)),
               (p0,p1)) for p0 in ls0 for p1 in ls1]
        apd.sort(lambda a, b: cmp(a[0], b[0]))
        #print apd
        for p in lps.get_particle_pairs():
            print IMP.core.get_distance(IMP.core.XYZR(p[0]), IMP.core.XYZR(p[1])),\
                  p[0].get_name(), p[1].get_name()
        for i in range(0,k):
            print "looking for", apd[i][0], apd[i][1][0].get_name(), apd[i][1][1].get_name()
            self.assertTrue(lps.get_contains(apd[i][1]))
        for i in range(k, len(apd)):
            print "not looking for", apd[i][0], apd[i][1][0].get_name(), apd[i][1][1].get_name()
            self.assertFalse(lps.get_contains(apd[i][1]))




if __name__ == '__main__':
    IMP.test.main()
