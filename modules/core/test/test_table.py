import IMP
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Tests for children refiner"""


    def test_set_position(self):
        """Make sure the Table PR works"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        print "hello"
        m= IMP.Model()
        p= IMP.Particle(m)
        tpr= IMP.core.TableRefiner()
        ps= []
        ps.append(p)
        tpr.add_particle(p, ps)
        self.assertTrue(tpr.get_can_refine(p))
        self.assertEqual(tpr.get_refined(p)[0], p)
        tpr.remove_particle(p)
        self.assertFalse(tpr.get_can_refine(p))




if __name__ == '__main__':
    IMP.test.main()
