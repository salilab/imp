import unittest
import os,math
import IMP
import IMP.em
import IMP.test
import IMP.core

class KernelParametersTest(IMP.test.TestCase):
    """Class to test Gaussian kernel parameters"""

    def test_for_resolution_of_10(self):
        self.assertAlmostEqual(self.kp.get_rsig(),4.24660921,0.001)
        self.assertAlmostEqual(self.kp.get_rsig()*self.kp.get_rsig(),
                               self.kp.get_rsigsq(),0.001)
        self.assertAlmostEqual(self.kp.get_timessig(),3,0.001)
        self.assertAlmostEqual(1./self.kp.get_rsigsq(),
                               self.kp.get_inv_rsigsq(),0.001)
        self.assertAlmostEqual(self.kp.get_sq2pi3(),1./math.sqrt(8.*math.pi*math.pi*math.pi))
        self.assertAlmostEqual(self.kp.get_rnormfac(),self.kp.get_sq2pi3()/76.549,0.001)
        self.assertAlmostEqual(self.kp.get_rkdist(),12.738,0.001)
        self.assertAlmostEqual(self.kp.get_lim(),0.011108,0.001)
    def test_for_resolution_of_10_and_radius_of_1(self):
        rad=1.
        self.kp.set_params(rad)
        kp=self.kp.get_params(rad)
        print dir(kp)
        print type(kp)
        self.assertAlmostEqual(kp.get_vsig(),1./math.sqrt(2)*rad,0.001)
        self.assertAlmostEqual(kp.get_vsig()*kp.get_vsig(),
                             kp.get_vsigsq(),0.001)
        self.assertAlmostEqual(0.5/(kp.get_vsigsq()+self.kp.get_rsigsq()),
                               kp.get_inv_sigsq(),0.001)
        self.assertAlmostEqual(kp.get_kdist(),12.738,0.001)
    def test_get_parameters_by_radius(self):
        rad=[1.2,1.5,2.,2.2]
        for r in rad:
            self.kp.set_params(r)
        for r in rad:
            self.assertEqual(self.kp.are_params_set(r),True)
            self.assertEqual(self.kp.are_params_set(r+0.1),False)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)
        self.kp=IMP.em.KernelParameters(10.)
if __name__ == '__main__':
    unittest.main()
