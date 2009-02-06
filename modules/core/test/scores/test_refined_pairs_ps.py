import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.core

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Checking refine once pair score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        pp= IMP.Particle()
        m.add_particle(pp)
        hpp= IMP.core.HierarchyDecorator.create(pp)
        c=[]
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            hp= IMP.core.HierarchyDecorator.create(p)
            hpp.add_child(hp)
            c.append(p)
        pr= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        cps= IMP.test.ConstPairScore(1)
        rps= IMP.core.RefinedPairsPairScore(pr, cps)
        self.assertEqual(rps.evaluate(pp, pp, None), 100)





if __name__ == '__main__':
    unittest.main()
