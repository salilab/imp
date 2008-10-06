import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.core

class Test(IMP.test.TestCase):
    """Tests for children refiner"""


    def test_set_position(self):
        """Make sure the Children refiner works"""
        IMP.set_log_level(IMP.VERBOSE)
        print "hello"
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
        pr= IMP.misc.ChildrenParticleRefiner()
        cs= pr.get_refined(pp)
        self.assertEqual(len(cs), len(c))
        for i in range(0, len(cs)):
            self.assertEqual(c[i].get_index().get_index(),
                             cs[i].get_index().get_index())



if __name__ == '__main__':
    unittest.main()
