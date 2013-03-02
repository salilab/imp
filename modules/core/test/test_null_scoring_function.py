import IMP
import IMP.test
import IMP.core
import IMP.container
import os

class Tests(IMP.test.TestCase):
    def test_cg_woods_func(self):
        """Check monte carlo with a null scoring function"""
        m= IMP.Model()
        n=10
        print 1
        ds= IMP.core.create_xyzr_particles(m, n, 2)
        mc= IMP.core.MonteCarlo(m)
        mv= IMP.core.BallMover(ds, 1.0)
        mc.add_mover(mv)
        mc.set_scoring_function([])
        mc.optimize(100)


if __name__ == '__main__':
    IMP.test.main()
