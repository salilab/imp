#!/usr/bin/env python

#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Weight
from IMP.isd import WeightMover

#unit testing framework
import IMP.test

class TestWeightMover(IMP.test.TestCase):
    """tests weight setup"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.kernel.Model()
        self.w = Weight.setup_particle(IMP.kernel.Particle(self.m))
        self.w.set_weights_are_optimized(True)
        self.w.add_weight()
        self.w.add_weight()
        self.wm= WeightMover(self.w,0.1)
        self.mc=IMP.core.MonteCarlo(self.m)
        self.mc.set_return_best(False)
        self.mc.set_kt(1.0)
        self.mc.add_mover(self.wm)
        

    def test_run(self):
        "Test weight mover mc run"
        self.setUp()
        for n in range(5):
           for j in range(10):
             self.mc.optimize(10)
             ws=self.w.get_weights()
             sum=0
             for k in range(self.w.get_number_of_states()):
                 sum+=self.w.get_weight(k)
             self.assertAlmostEqual(sum,1.0,delta=0.0000001)     
           self.w.add_weight()


if __name__ == '__main__':
    IMP.test.main()
