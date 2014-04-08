import IMP.kernel
import IMP.test
import IMP.core

from math import *


class NormalMoverTest(IMP.test.TestCase):

    def test_zeros(self):
        m = IMP.kernel.Model()
        pi = m.add_particle("p")
        xyzr = IMP.core.XYZR.setup_particle(m, pi)
        rb = IMP.core.RigidBody.setup_particle(
            m, pi, IMP.algebra.ReferenceFrame3D())
        rb.set_coordinates_are_optimized(True)
        print "0"
        rbmt0 = IMP.core.RigidBodyMover(m, pi, 0, 1)
        rbmt0.propose()
        rbmt0.accept()
        print "1"
        rbmr0 = IMP.core.RigidBodyMover(m, pi, 1, 0)
        rbmr0.propose()
        rbmr0.accept()
if __name__ == '__main__':
    IMP.test.main()
