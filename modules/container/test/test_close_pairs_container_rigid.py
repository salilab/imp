from __future__ import print_function
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random


class Tests(IMP.test.TestCase):

    def test_rigid(self):
        """Test ClosePairContainer with rigid finder"""
        m = IMP.Model()
        m.set_log_level(IMP.TERSE)
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 10, 10))
        slack = 1

        def create_rb():
            rbp = IMP.Particle(m)
            ps = []
            for i in range(0, 10):
                p = m.add_particle("p%d" % i)
                d = IMP.core.XYZR.setup_particle(m,
                    p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 3))
                ps.append(p)
            rb = IMP.core.RigidBody.setup_particle(rbp, ps)
            rb.set_coordinates_are_optimized(True)
            return (rb, ps)
        (rb0, ps0) = create_rb()
        (rb1, ps1) = create_rb()
        lsc = IMP.container.ListSingletonContainer(m, ps0 + ps1)
        nbl = IMP.container.ClosePairContainer(
            lsc,
            0,
            IMP.core.RigidClosePairsFinder(),
            slack)
        self.assertEqual(nbl.get_slack(), slack)
        # nbl.set_log_level(IMP.VERBOSE)
        m.update()
        for pi in nbl.get_contents():
            self.assertNotEqual(
                IMP.core.RigidMember(m, pi[0]).get_rigid_body(),
                IMP.core.RigidMember(m, pi[1]).get_rigid_body())

        def test_empty():
            for l0 in ps0:
                for l1 in ps1:
                    self.assertGreaterEqual(
                        IMP.core.get_distance(IMP.core.XYZR(m, l0),
                                              IMP.core.XYZR(m, l1)), 0)

        def test_not_empty():
            for l0 in ps0:
                for l1 in ps1:
                    if IMP.core.get_distance(IMP.core.XYZR(m, l0),
                                             IMP.core.XYZR(m, l1)) < 2.5 * slack:
                        return
            self.assert_(False)

        print(ps0, ps1)
        rbm0 = IMP.core.RigidBodyMover(m, rb0, 5, 1)
        rbm1 = IMP.core.RigidBodyMover(m, rb1, 5, 1)
        tested = False
        for i in range(0, 100):
            rbm0.propose()
            rbm0.accept()
            rbm1.propose()
            rbm1.accept()
            m.update()
            if len(nbl.get_contents()) == 0:
                test_empty()
                print("tested")
                tested = True
            else:
                print("collision")
                test_not_empty()
                # print "collision"
        assert tested, "Never cleared"
if __name__ == '__main__':
    IMP.test.main()
