import IMP
import IMP.test
import IMP.misc
import IMP.core
import IMP.algebra

rk = IMP.FloatKey("radius")

class Tests(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Check that LowestRefinedPairScore returns the lowest"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        pp0= IMP.Particle(m)
        pp1= IMP.Particle(m)
        hpp= [IMP.core.Hierarchy.setup_particle(pp0),
              IMP.core.Hierarchy.setup_particle(pp1)]
        ds=[[],[]]
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
            ds[i%2].append(d)
            hp= IMP.core.Hierarchy.setup_particle(p)
            hpp[i%2].add_child(hp)
        # could be more clever
        md= 1000000
        for p0 in ds[0]:
            for p1 in ds[1]:
                d= IMP.core.get_distance(p0, p1)
                if d < md: md=d
        pr= IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())
        cps= IMP.core.Linear(0,1)
        dps= IMP.core.DistancePairScore(cps)
        rps= IMP.misc.LowestRefinedPairScore(pr, dps)
        self.assertAlmostEqual(rps.evaluate((pp0, pp1),
                                            None), .9*md, delta=1.1*md)





if __name__ == '__main__':
    IMP.test.main()
