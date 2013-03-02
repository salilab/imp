import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example

class Tests(IMP.test.TestCase):
    """Test the symmetry restraint"""
    def test_symmetry(self):
        """Test example filter"""
        m=IMP.Model()
        ps=[IMP.Particle(m) for i in xrange(10)]
        sft= IMP.example.ExampleSubsetFilterTable(3, [ps[0], ps[4]])
        self.assertEqual(sft.get_subset_filter(IMP.domino.Subset([ps[0], ps[1], ps[6]]), []),
                         None)
        self.assertEqual(sft.get_subset_filter(IMP.domino.Subset([ps[0], ps[1], ps[6], ps[4]]),
                                               [IMP.domino.Subset([ps[0], ps[4], ps[3]])]),
                         None)
        sf= sft.get_subset_filter(IMP.domino.Subset([ps[0], ps[1], ps[6], ps[4]]),
                                               [])
        self.assertEqual(sf.get_is_ok(IMP.domino.Assignment([3,3,3,3])), True)
        self.assertEqual(sf.get_is_ok(IMP.domino.Assignment([0,4,8,12])), False)
        ls= list(IMP.domino.Subset([ps[0], ps[1], ps[6], ps[4]]))
        ss= IMP.domino.Assignment([0 for l in ls])
        ssl=[x for x in ss]
        ssl[ls.index(ps[0])]=6
        ss= IMP.domino.Assignment(ssl)
        self.assertEqual(sf.get_is_ok(ss), False)
if __name__ == '__main__':
    IMP.test.main()
