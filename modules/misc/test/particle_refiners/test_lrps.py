import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.core

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Check that LowestRefinedPairScore returns the lowest"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        pp0= IMP.Particle()
        m.add_particle(pp0)
        pp1= IMP.Particle()
        m.add_particle(pp1)
        hpp= [IMP.core.HierarchyDecorator.create(pp0),
              IMP.core.HierarchyDecorator.create(pp1)]
        ds=[[],[]]
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d= IMP.core.XYZDecorator.create(p)
            d.set_coordinates(IMP.random_vector_in_box())
            ds[i%2].append(d)
            hp= IMP.core.HierarchyDecorator.create(p)
            hpp[i%2].add_child(hp)
        # could be more clever
        md= 1000000
        for p0 in ds[0]:
            for p1 in ds[1]:
                d= IMP.core.distance(p0, p1)
                if d < md: md=d
        pr= IMP.misc.ChildrenParticleRefiner()
        cps= IMP.core.Linear(0,1)
        dps= IMP.core.DistancePairScore(cps)
        rps= IMP.misc.LowestRefinedPairScore(pr, dps)
        self.assertInTolerance(rps.evaluate(pp0, pp1, None), .9*md, 1.1*md)





if __name__ == '__main__':
    unittest.main()
