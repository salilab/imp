import IMP
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of FixedRefiner"""
        m = IMP.Model()
        ps = IMP.core.create_xyzr_particles(m, 10, 1)
        fpr = IMP.core.FixedRefiner(ps)
        fpr.set_name("foo")
        dump = pickle.dumps(fpr)
        newfpr = pickle.loads(dump)
        self.assertEqual(newfpr.get_refined(ps[0]), ps)
        self.assertEqual(newfpr.get_name(), "foo")

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of FixedRefiner via polymorphic pointer"""
        m = IMP.Model()
        ps = IMP.core.create_xyzr_particles(m, 10, 1)
        fpr = IMP.core.FixedRefiner(ps)
        fpr.set_name("foo")
        cr = IMP.core.CentroidOfRefined(fpr)
        dump = pickle.dumps(cr)
        newcr = pickle.loads(dump)
        newfpr = newcr.get_refiner()
        self.assertEqual(newfpr.get_refined(ps[0]), ps)
        self.assertEqual(newfpr.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
