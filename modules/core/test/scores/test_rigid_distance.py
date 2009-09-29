import unittest
import IMP
import IMP.test
import IMP.misc
import IMP.core
import IMP.helper

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""



    def test_rops(self):
        """Checking rigid distance pair score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        p1= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        IMP.atom.add_radii(p0)
        IMP.atom.add_radii(p1)
        s0= IMP.helper.create_rigid_body(p0)
        m.add_score_state(s0)
        s1= IMP.helper.create_rigid_body(p1)
        m.add_score_state(s1)
        IMP.core.RigidBody(p0.get_particle()).set_transformation(IMP.algebra.Transformation3D(IMP.algebra.identity_rotation(), IMP.algebra.Vector3D(100,100,100)))
        sdps= IMP.core.SphereDistancePairScore(IMP.core.Linear(0,1))
        rdps= IMP.core.RigidBodyDistancePairScore(sdps, IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        v= rdps.evaluate(p0.get_particle(), p1.get_particle(), None)
        dm= 1000000
        bp=None
        for l0 in IMP.core.get_leaves(p0):
            for l1 in IMP.core.get_leaves(p1):
                d= sdps.evaluate(l0.get_particle(), l1.get_particle(), None)
                if d< dm:
                    dm=d
        self.assertInTolerance(v, dm, .1)


    def test_rops(self):
        """Checking rigid distance pair score against one"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        p1= IMP.Particle(m)
        IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(100,100,100), 3))
        IMP.atom.add_radii(p0)
        s0= IMP.helper.create_rigid_body(p0)
        m.add_score_state(s0)
        sdps= IMP.core.SphereDistancePairScore(IMP.core.Linear(0,1))
        rdps= IMP.core.RigidBodyDistancePairScore(sdps, IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        v= rdps.evaluate(p0.get_particle(), p1, None)
        dm= 1000000
        bp=None
        for l0 in IMP.core.get_leaves(p0):
            d= sdps.evaluate(l0.get_particle(), p1, None)
            if d< dm:
                dm=d
        self.assertInTolerance(v, dm, .1)


if __name__ == '__main__':
    unittest.main()
