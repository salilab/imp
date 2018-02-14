from __future__ import print_function
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra

from math import *

def get_axis_and_angle(q):
    angle = 2 * acos(q[0])
    if angle != 0:
        x = q[1] / sqrt(1-q[0]*q[0])
        y = q[2] / sqrt(1-q[0]*q[0])
        z = q[3] / sqrt(1-q[0]*q[0])
        return (angle,(x,y,z))
    else:
        return (0,None)

class NormalMoverTest(IMP.test.TestCase):

    def test_zeros(self):
        m = IMP.Model()
        pi = m.add_particle("p")
        xyzr = IMP.core.XYZR.setup_particle(m, pi)
        rb = IMP.core.RigidBody.setup_particle(
            m, pi, IMP.algebra.ReferenceFrame3D())
        rb.set_coordinates_are_optimized(True)
        print("0")
        rbmt0 = IMP.core.RigidBodyMover(m, pi, 0, 1)
        rbmt0.propose()
        rbmt0.accept()
        print("1")
        rbmr0 = IMP.core.RigidBodyMover(m, pi, 1, 0)
        rbmr0.propose()
        rbmr0.accept()

    def setup_mv_mover_test(self,trans=None,rot=None):
        mdl = IMP.Model()
        mol=IMP.atom.Hierarchy(IMP.Particle(mdl))
        root=IMP.atom.Hierarchy(IMP.Particle(mdl))
        root.add_child(mol)
        ps=[]
        for c in [(100,100,100),(5,100,100),(10,5,100)]:
            p=IMP.Particle(mdl)
            dr=IMP.core.XYZR.setup_particle(p)
            dr.set_coordinates(c)
            dr.set_radius(1.0)
            IMP.atom.Mass.setup_particle(p,1.0)
            h=IMP.atom.Hierarchy(p)
            mol.add_child(h)
            ps.append(p)

        rb = IMP.atom.create_rigid_body(IMP.atom.get_leaves(mol))
        rb.set_coordinates_are_optimized(True)
        rb_mover = IMP.core.RigidBodyMover(rb.get_model(),rb,trans,rot)

        ssps = IMP.core.SoftSpherePairScore(1.0)
        lsa = IMP.container.ListSingletonContainer(mdl)
        lsa.add(IMP.get_indexes(IMP.atom.get_leaves(root)))
        rbcpf = IMP.core.RigidClosePairsFinder()
        cpc = IMP.container.ClosePairContainer(lsa, 0.0, rbcpf, 10.0)
        evr = IMP.container.PairsRestraint(ssps, cpc)

        sf = IMP.core.RestraintsScoringFunction([evr])
        mc = IMP.core.MonteCarlo(mdl)
        mc.set_scoring_function(sf)

        sm = IMP.core.SerialMover([rb_mover])
        mc.add_mover(sm)
        mc.set_return_best(False)
        mc.set_kt(1.0)
        return mdl,root,mc,rb

    def test_mc_mover_translate(self):
        mdl,root,mc,rb=self.setup_mv_mover_test(1.0,0.0)
        r0=rb.get_coordinates()
        rot0=rb.get_rotation()
        mc.optimize(1)
        r1=rb.get_coordinates()
        rot1=rb.get_rotation()
        transm=(r1-r0).get_magnitude()
        q=(rot1/rot0).get_quaternion()
        self.assertAlmostEqual(get_axis_and_angle(q)[0],0.0,places=6)
        self.assertLessEqual(transm,1.0)

    def test_mc_mover_rotate(self):
        for i in range(100):
            mdl,root,mc,rb=self.setup_mv_mover_test(0.0,1.0)
            r0=rb.get_coordinates()
            rot0=rb.get_rotation()
            mc.optimize(1)
            r1=rb.get_coordinates()
            rot1=rb.get_rotation()
            transm=(r1-r0).get_magnitude()
            q=(rot1/rot0).get_quaternion()
            self.assertLessEqual(abs(get_axis_and_angle(q)[0]),1.0)
            self.assertAlmostEqual(transm,0.0,places=6)

    def test_mc_mover_rotate_transalte(self):
        mdl,root,mc,rb=self.setup_mv_mover_test(1.0,1.0)
        r0=rb.get_coordinates()
        rot0=rb.get_rotation()
        mc.optimize(1)
        r1=rb.get_coordinates()
        rot1=rb.get_rotation()
        transm=(r1-r0).get_magnitude()
        q=(rot1/rot0).get_quaternion()
        self.assertLessEqual(abs(get_axis_and_angle(q)[0]),1.0)
        self.assertLessEqual(transm,1.0)



if __name__ == '__main__':
    IMP.test.main()
