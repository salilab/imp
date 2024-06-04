import IMP
import IMP.core
import IMP.test
import pickle


class Tests(IMP.test.TestCase):
    """Test ConstantRestraint"""

    def test_pickle(self):
        """Test (un-)pickle of ConstantRestraint"""
        m = IMP.Model()
        r = IMP.core.ConstantRestraint(m, 42.0)
        r.set_name("foo")
        dump = pickle.dumps(r)

        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(False), 42.0, delta=1e-3)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of ConstantRestraint via polymorphic pointer"""
        m = IMP.Model()
        r = IMP.core.ConstantRestraint(m, 42.0)
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        dump = pickle.dumps(sf)

        newsf = pickle.loads(dump)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(False), 42.0, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
