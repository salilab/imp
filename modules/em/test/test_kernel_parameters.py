import os,math
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test Gaussian kernel parameters"""

    def test_for_resolution_of_10(self):
        self.assertAlmostEqual(self.kp.get_rsig(),4.24660921, delta=0.001)
        self.assertAlmostEqual(self.kp.get_rsig()*self.kp.get_rsig(),
                               self.kp.get_rsigsq(), delta=0.001)
        self.assertAlmostEqual(self.kp.get_timessig(),3, delta=0.001)
        self.assertAlmostEqual(1./(2.*self.kp.get_rsigsq()),
                               self.kp.get_inv_rsigsq(), delta=0.001)
        self.assertAlmostEqual(self.kp.get_sq2pi3(),1./math.sqrt(8.*math.pi*math.pi*math.pi), delta=0.001)
        self.assertAlmostEqual(self.kp.get_rnormfac(),self.kp.get_sq2pi3()/76.549, delta=0.001)
        self.assertAlmostEqual(self.kp.get_rkdist(),12.739, delta=0.001)
        self.assertAlmostEqual(self.kp.get_lim(),0.011108, delta=0.001)

    def test_for_resolution_of_10_and_radius_of_1(self):
        rad=1.
        self.kp.set_params(rad)
        kp=self.kp.get_params(rad)
        print dir(kp)
        print type(kp)
        self.assertAlmostEqual(kp.get_vsig(),1./math.sqrt(2)*rad, delta=0.001)
        self.assertAlmostEqual(kp.get_vsig()*kp.get_vsig(),
                             kp.get_vsigsq(), delta=0.001)
        self.assertAlmostEqual(0.5/(kp.get_vsigsq()+self.kp.get_rsigsq()),
                               kp.get_inv_sigsq(), delta=0.001)
        self.assertAlmostEqual(kp.get_kdist(),12.9152, delta=0.001)
    def test_get_parameters_by_radius(self):
        rad=[1.2,1.5,2.,2.2]
        for r in rad:
            self.kp.set_params(r)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.VERBOSE)
        self.kp=IMP.em.KernelParameters(10.)
if __name__ == '__main__':
    IMP.test.main()
