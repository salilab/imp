import IMP.test
import IMP.saxs
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of Profile"""
        p = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))
        p.set_name("foo")
        self.assertAlmostEqual(p.get_min_q(), 0.04138, delta=1e-4)
        self.assertAlmostEqual(p.get_max_q(), 0.49836, delta=1e-4)
        self.assertAlmostEqual(p.get_delta_q(), 0.0023315, delta=1e-4)
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)
        self.assertEqual(p.get_name(), "foo")
        self.assertAlmostEqual(newp.get_min_q(), 0.04138, delta=1e-4)
        self.assertAlmostEqual(newp.get_max_q(), 0.49836, delta=1e-4)
        self.assertAlmostEqual(newp.get_delta_q(), 0.0023315, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
