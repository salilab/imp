import unittest
import IMP
import IMP.test
import IMP.core

class Test(IMP.test.TestCase):
    """Tests for children refiner"""


    def test_set_position(self):
        """Make sure the Table PR works"""
        IMP.set_log_level(IMP.VERBOSE)
        print "hello"
        m= IMP.Model()
        p= IMP.Particle(m)
        tpr= IMP.core.TableParticleRefiner()
        ps= IMP.Particles()
        ps.append(p)
        tpr.add_particle(p, ps)
        self.assert_(tpr.get_can_refine(p))
        self.assert_(tpr.get_refined(p)[0] == p)
        tpr.remove_particle(p)
        self.assert_(not tpr.get_can_refine(p))




if __name__ == '__main__':
    unittest.main()
