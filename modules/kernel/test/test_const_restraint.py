import IMP
import IMP.test
import pickle


class Tests(IMP.test.TestCase):
    """Test ConstRestraint"""

    def test_pickle(self):
        """Test (un-)pickle of ConstRestraint"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        r = IMP._ConstRestraint(m, [p1, p2], 42.0)
        r.set_name("foo")
        dump = pickle.dumps(r)

        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.get_value(), 42.0, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
