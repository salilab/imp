import IMP.misc
import IMP
import IMP.test


class Tests(IMP.test.TestCase):

    """Tests for WLC unary function"""

    def test_wlc(self):
        """Test that decay has the right halflife"""
        m = IMP.Model()
        ps = [m.add_particle("P%d" % i) for i in range(0, 100)]
        ap = []
        for i in range(0, len(ps)):
            for j in range(0, i):
                ap.append((ps[i], ps[j]))
        cf = IMP.core.CoinFlipPairPredicate(.9)
        dos = IMP.misc.DecayPairContainerOptimizerState(m, cf, ap, "decay")
        dos.set_log_level(IMP.TERSE)
        np = len(ap)
        dos.set_period(1)
        for i in range(0, 6):
            dos.update()
        out = dos.get_output_container()
        nout = len(out.get_contents())
        # probabilistic
        self.assertAlmostEqual(nout, .53 * np, delta=100)

if __name__ == '__main__':
    IMP.test.main()
