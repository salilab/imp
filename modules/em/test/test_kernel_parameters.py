import math
import IMP
import IMP.em
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    """Class to test Gaussian kernel parameters"""

    def test_for_resolution_of_10(self):
        self.assertAlmostEqual(self.kp.get_rsig(), 2.1233, delta=0.001)
        self.assertAlmostEqual(self.kp.get_rsig() * self.kp.get_rsig(),
                               self.kp.get_rsigsq(), delta=0.001)
        self.assertAlmostEqual(self.kp.get_timessig(), 3, delta=0.001)
        self.assertAlmostEqual(1. / (2. * self.kp.get_rsigsq()),
                               self.kp.get_inv_rsigsq(), delta=0.001)
        self.assertAlmostEqual(
            self.kp.get_sq2pi3(),
            1. / math.sqrt(8. * math.pi * math.pi * math.pi),
            delta=0.001)
        self.assertAlmostEqual(
            self.kp.get_rnormfac(),
            self.kp.get_sq2pi3() / 76.549,
            delta=0.01)
        self.assertAlmostEqual(self.kp.get_rkdist(), 6.3699, delta=0.001)
        self.assertAlmostEqual(self.kp.get_lim(), 0.011108, delta=0.001)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)
        self.kp = IMP.em.KernelParameters(10.)

    def test_pickle(self):
        """Test (un-)pickle of KernelParameters"""
        k = IMP.em.KernelParameters(10.)
        dump = pickle.dumps(k)
        newk = pickle.loads(dump)
        self.assertAlmostEqual(k.get_rsig(), newk.get_rsig(), delta=1e-4)
        self.assertAlmostEqual(k.get_sq2pi3(), newk.get_sq2pi3(), delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
