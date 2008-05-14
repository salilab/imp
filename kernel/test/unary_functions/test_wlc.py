import unittest
import IMP
import IMP.test


class WLCTests(IMP.test.TestCase):
    """Tests for WLC unary function"""

    def test_wlc(self):
        """Test that the WormLikeChain values are sane"""
        wlc= IMP.WormLikeChain(200, 3.4)
        self.check_unary_function_min(wlc, 0, 250, .5, 0)
        self.check_unary_function_deriv(wlc, 0, 250, .5)

        self.assert_(wlc.evaluate_deriv(180)[1] > 4.2)

if __name__ == '__main__':
    unittest.main()
