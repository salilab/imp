import IMP
import IMP.test
import IMP.em
import numpy as np


def add_logabssumprodexp(a, b):
    s = - np.inf
    ss = -1
    for i in range(len(a)):
        s, ss = IMP.em.bayesem3d_get_logabssumprodexp(s, a[i], ss, b[i])
    return s, ss


def add_logsumprodexp(a, b):
    s = 0
    for i in range(len(a)):
        s = IMP.em.bayesem3d_get_logsumprodexp(s, a[i], 1., b[i])
    return s


class Tests(IMP.test.TestCase):
    """Stable LogSumExp Testing"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.x = np.arange(10)
        indices = np.random.choice(np.arange(self.x.size),
                                   replace=False,
                                   size=int(self.x.size * 0.2))
        self.x[indices] = 0
        self.w = 2 * np.random.rand(self.x.size) - 1
        self.w2 = np.random.rand(self.x.size)

    def test_logabssumexpprod(self):
        """Compare logabssumprodexp to naive implementation"""
        sum_0 = np.sum(self.w * np.exp(self.x))
        logsum, sign = add_logabssumprodexp(self.x, self.w)
        sum_1 = sign * np.exp(logsum)
        self.assertAlmostEqual(sum_0, sum_1, delta=1e-6)

    def test_logsumexpprod(self):
        """Compare logsumprodexp to naive implementation"""
        sum_0 = np.sum(self.w2 * np.exp(self.x))
        logsum = add_logsumprodexp(self.x, self.w2)
        self.assertAlmostEqual(sum_0, np.exp(logsum) - 1, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
