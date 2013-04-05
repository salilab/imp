import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random

class Tests(IMP.test.TestCase):

    def test_rigid(self):
        """Test ClosePairContainer with rigid finder"""
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        slack = 1
        def create_rb():
            rbp= IMP.Particle(m)
            ps= []
            for i in range(0,10):
                p = IMP.Particle(m)
                d= IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 3))
                ps.append(p)
            return (IMP.core.RigidBody.setup_particle(rbp, ps), ps)
        (rb0, ps0)= create_rb()
        (rb1, ps1)= create_rb()
        lsc= IMP.container.ListSingletonContainer(ps0+ps1)
        nbl= IMP.container.ClosePairContainer(lsc, 0, IMP.core.RigidClosePairsFinder(), slack)
        #nbl.set_log_level(IMP.base.VERBOSE)
        m.update()
        for p in nbl.get_particle_pairs():
            self.assertNotEqual(IMP.core.RigidMember(p[0]).get_rigid_body(),
                                IMP.core.RigidMember(p[1]).get_rigid_body())

        def test_empty():
            for l0 in ps0:
                for l1 in ps1:
                    self.assertGreaterEqual(
                           IMP.core.get_distance(IMP.core.XYZR(l0),
                                                 IMP.core.XYZR(l1)), 0)
        def test_not_empty():
            for l0 in ps0:
                for l1 in ps1:
                    if IMP.core.get_distance(IMP.core.XYZR(l0),
                                             IMP.core.XYZR(l1)) < 2*slack:
                        return
            self.assert_(False)

        rbm0= IMP.core.RigidBodyMover(rb0, 5, 1)
        rbm1= IMP.core.RigidBodyMover(rb1, 5, 1)
        tested = False
        for i in range(0,100):
            rbm0.propose()
            rbm0.accept()
            rbm1.propose()
            rbm1.accept()
            m.update()
            if nbl.get_number_of_particle_pairs()==0:
                test_empty()
                print "tested"
                tested = True
            else:
                print "collision"
                test_not_empty()
                #print "collision"
        assert tested, "Never cleared"
if __name__ == '__main__':
    IMP.test.main()
