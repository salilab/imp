import IMP
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Tests for children refiner"""


    def test_set_position(self):
        """Make sure the Children refiner works"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        print "hello"
        m= IMP.Model()
        pp= IMP.Particle(m)
        hpp= IMP.core.Hierarchy.setup_particle(pp)
        c=[]
        for i in range(0,10):
            p= IMP.Particle(m)
            hp= IMP.core.Hierarchy.setup_particle(p)
            hpp.add_child(hp)
            c.append(p)
        pr= IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())
        cs= pr.get_refined(pp)
        self.assertEqual(len(cs), len(c))
        for i in range(0, len(cs)):
            self.assertEqual(c[i].get_name(),
                             cs[i].get_name())



if __name__ == '__main__':
    IMP.test.main()
