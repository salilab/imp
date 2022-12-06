import IMP
import IMP.test
import IMP.atom
import IMP.rotamer
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of RotamerAngleTuple"""
        t = IMP.rotamer.RotamerAngleTuple(1.0, 2.0, 3.0, 4.0, 0.5)
        dump = pickle.dumps(t)
        newt = pickle.loads(dump)
        self.assertAlmostEqual(newt.get_chi1(), 1.0, delta=1e-4)
        self.assertAlmostEqual(newt.get_chi2(), 2.0, delta=1e-4)
        self.assertAlmostEqual(newt.get_chi3(), 3.0, delta=1e-4)
        self.assertAlmostEqual(newt.get_chi4(), 4.0, delta=1e-4)
        self.assertAlmostEqual(newt.get_probability(), 0.5, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
