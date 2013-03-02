import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_global_min2(self):
        """Test permutation particle states"""
        dps= IMP.domino.IndexStates(10)
        pps= IMP.domino.PermutationStates(dps)
        all=[pps.get_inner_state(i) for i in range(0,10)]
        print all
        for i in range(0,10):
            self.assertIn(i, all)
if __name__ == '__main__':
    IMP.test.main()
