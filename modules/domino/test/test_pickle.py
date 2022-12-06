import IMP
import IMP.test
import IMP.domino
import pickle


class Tests(IMP.test.TestCase):

    def test_assignment_pickle(self):
        """Test (un-)pickle of Assignment"""
        a = IMP.domino.Assignment([0])
        dump = pickle.dumps(a)
        newa = pickle.loads(dump)
        self.assertEqual(newa, a)

    def test_order_pickle(self):
        """Test (un-)pickle of Order"""
        m = IMP.Model()
        ps = [IMP.Particle(m) for p in range(10)]
        outer = IMP.domino.Subset([ps[0], ps[1]])
        a = IMP.domino.Order(outer, [ps[0], ps[1]])
        dump = pickle.dumps(a)
        newa = pickle.loads(dump)
        self.assertEqual(newa, a)

    def test_slice_pickle(self):
        """Test (un-)pickle of Slice"""
        m = IMP.Model()
        ps = [IMP.Particle(m) for p in range(10)]
        outer = IMP.domino.Subset(ps)
        inner = IMP.domino.Subset([ps[3], ps[5]])
        a = IMP.domino.Slice(outer, inner)
        dump = pickle.dumps(a)
        newa = pickle.loads(dump)
        self.assertEqual(newa, a)


if __name__ == '__main__':
    IMP.test.main()
