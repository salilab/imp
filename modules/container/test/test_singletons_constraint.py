import IMP
import IMP.test
import IMP.container
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)serialize of SingletonsConstraint"""
        m = IMP.Model()
        p1 = m.add_particle("p1")
        lpc = IMP.container.ListSingletonContainer(m, [p1])
        ss = IMP.container.SingletonsConstraint(None, None, lpc)
        dump = pickle.dumps(ss)
        newss = pickle.loads(dump)


if __name__ == '__main__':
    IMP.test.main()
