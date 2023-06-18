import IMP
import IMP.test
import IMP.core
import math
import io
import pickle


def _cosfunc(val, force_constant, periodicity, phase):
    """Python implementation of cosine function and first derivative"""
    score = abs(force_constant) \
        - force_constant * math.cos(periodicity * val + phase)
    deriv = force_constant * periodicity * math.sin(periodicity * val + phase)
    return score, deriv


class Tests(IMP.test.TestCase):

    """Tests for cosine unary function"""

    def test_values(self):
        """Test that cosine values are correct"""
        for force_constant in (0, 10.0, -5.0):
            for periodicity in (1, 2, 3, 4):
                for phase in (0.0, math.pi / 2.0, math.pi):
                    func = IMP.core.Cosine(force_constant, periodicity, phase)
                    func.set_was_used(True)
                    for i in range(15):
                        val = -math.pi + math.pi * 15.0 / (i + 1.0)
                        expscore, expderiv = _cosfunc(val, force_constant,
                                                      periodicity, phase)
                        score, deriv = func.evaluate_with_derivative(val)
                        scoreonly = func.evaluate(val)
                        self.assertAlmostEqual(score, scoreonly, delta=1e-4)
                        self.assertAlmostEqual(expscore, score, delta=0.1)
                        self.assertAlmostEqual(expderiv, deriv, delta=0.1)

    def test_pickle(self):
        """Test (un-)pickle of Cosine"""
        func = IMP.core.Cosine(10.0, 2, 0.0)
        func.set_name('foo')
        self.assertAlmostEqual(func.evaluate(4.0), 11.455, delta=0.01)
        dump = pickle.dumps(func)
        del func
        f = pickle.loads(dump)
        self.assertEqual(f.get_name(), 'foo')
        self.assertAlmostEqual(f.evaluate(4.0), 11.455, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
