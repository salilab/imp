import unittest
import IMP
import IMP.test
import IMP.misc

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Make sure that bond cover coordinates are correct"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        pp= IMP.Particle()
        m.add_particle(pp)
        hpp= IMP.HierarchyDecorator.create(pp)
        c=[]
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            hp= IMP.HierarchyDecorator.create(p)
            hpp.add_child(hp)
            c.append(p)
        pr= IMP.misc.ChildrenParticleRefiner()
        cps= IMP.test.ConstPairScore(1)
        rps= IMP.misc.RefineOncePairScore(pr, cps)
        self.assertEqual(rps.evaluate(pp, pp, None), 100)





if __name__ == '__main__':
    unittest.main()
