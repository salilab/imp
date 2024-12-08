import IMP
import IMP.test
import pickle


class Tests(IMP.test.TestCase):
    """Test ConstOptimizer"""

    def test_pickle(self):
        """Test (un-)pickle of ConstOptimizer"""
        m = IMP.Model()
        opt = IMP._ConstOptimizer(m)
        opt.set_name("foo")
        opt.set_stop_on_good_score(True)
        dump = pickle.dumps(opt)

        newopt = pickle.loads(dump)
        self.assertEqual(newopt.get_name(), "foo")
        self.assertEqual(newopt.get_stop_on_good_score(), True)


if __name__ == '__main__':
    IMP.test.main()
