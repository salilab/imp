#!/usr/bin/env python

# imp general
import IMP
import IMP.core

# our project
from IMP.isd import Weight
from IMP.isd import WeightMover

# unit testing framework
import IMP.test


class TestWeightMover(IMP.test.TestCase):

    """tests weight setup"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.setup_system(2)

    def setup_system(self, nweights):
        self.m = IMP.Model()
        self.w = Weight.setup_particle(IMP.Particle(self.m), nweights)
        self.w.set_weights_are_optimized(True)
        self.wm = WeightMover(self.w, 0.1)
        self.mc = IMP.core.MonteCarlo(self.m)
        self.mc.set_scoring_function([])
        self.mc.set_return_best(False)
        self.mc.set_kt(1.0)
        self.mc.add_mover(self.wm)

    def test_run(self):
        "Test weight mover mc run"
        for n in range(5):
            self.setup_system(n + 2)
            for j in range(10):
                self.mc.optimize(10)
                ws = self.w.get_weights()
                wsum = sum([ws[i] for i in range(len(ws))])
                self.assertAlmostEqual(wsum, 1.0, delta=0.0000001)


if __name__ == '__main__':
    IMP.test.main()
