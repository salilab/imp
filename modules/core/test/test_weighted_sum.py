import IMP
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    def test_values(self):
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        for i in range(-10, 10):
            i = float(i)
            self.assertAlmostEqual(
                sf.evaluate(i), .3 * f1.evaluate(i) + .7 * f2.evaluate(i))
            score, deriv = sf.evaluate_with_derivative(i)
            score_sum = 0
            deriv_sum = 0
            for w, f in zip([.3, .7], [f1, f2]):
                s, d = f.evaluate_with_derivative(i)
                score_sum += w * s
                deriv_sum += w * d
            self.assertAlmostEqual(score, score_sum, delta=1e-4)
            self.assertAlmostEqual(deriv, deriv_sum, delta=1e-4)

    def test_update_functions(self):
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 2.)
        sf = IMP.core.WeightedSum([f1, f2], [.5, .5])
        self.assertAlmostEqual(sf.evaluate(0), 2., delta=1e-6)
        f2.set_k(1.)
        self.assertAlmostEqual(sf.evaluate(0), 1., delta=1e-6)

    def test_accessors(self):
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        self.assertAlmostEqual(sf.get_weight(0), .3)
        self.assertAlmostEqual(sf.get_weight(1), .7)
        self.assertAlmostEqual(sf.get_weights()[0], .3)
        sf.set_weights([.4, .6])
        self.assertAlmostEqual(sf.get_weight(0), .4)
        self.assertAlmostEqual(sf.get_weight(1), .6)

    def test_errors(self):
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        self.assertRaisesUsageException(IMP.core.WeightedSum,
                                        [f1], [1.])
        self.assertRaisesUsageException(IMP.core.WeightedSum,
                                        [f1, f2], [1.])
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        self.assertRaisesUsageException(sf.set_weights,
                                        [1.])


if __name__ == '__main__':
    IMP.test.main()
