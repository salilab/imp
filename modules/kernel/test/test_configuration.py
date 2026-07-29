import IMP
import IMP.test
import pickle

ik0 = IMP.IntKey("i0")


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un)pickle of Configuration"""
        m = IMP.Model()
        p = IMP.Particle(m)
        m.add_attribute(ik0, p, 42)

        conf = IMP.Configuration(m, "foo")

        m.set_attribute(ik0, p, 99)

        dump = pickle.dumps(conf)
        newconf = pickle.loads(dump)
        self.assertEqual(newconf.get_name(), "foo")

        self.assertEqual(m.get_attribute(ik0, p), 99)
        newconf.load_configuration()
        # Attribute should be restored to original value
        self.assertEqual(m.get_attribute(ik0, p), 42)


if __name__ == '__main__':
    IMP.test.main()
