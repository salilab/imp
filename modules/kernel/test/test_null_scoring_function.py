import IMP.kernel
import IMP.test
import os

class Tests(IMP.test.TestCase):
    def test_cg_woods_func(self):
        """Check that ScoringFunctionAdaptors create null scoring functions"""
        m= IMP.kernel.Model()
        n=10
        print 1
        opt = IMP.kernel._ConstOptimizer(m)
        opt.set_scoring_function([])
        opt.optimize(100)


if __name__ == '__main__':
    IMP.test.main()
