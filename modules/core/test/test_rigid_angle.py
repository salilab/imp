import IMP
import IMP.test
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Test the RigidBodyAnglePairScore"""

    def make_system(self):
        rf = IMP.algebra.ReferenceFrame3D()
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        b1 = IMP.core.RigidBody.setup_particle(p1, rf)
        b2 = IMP.core.RigidBody.setup_particle(p2, rf)
        uf = IMP.core.Linear(0., 1.)
        ps = IMP.core.RigidBodyAnglePairScore(uf)
        rs = IMP.core.PairRestraint(ps, (p1, p2))
        return m, p1, p2, b1, b2, ps, rs

    def test_angle(self):
        """Test RigidBodyAnglePairScore evaluation"""
        m, p1, p2, b1, b2, ps, rs = self.make_system()
        # At setup, angle should be zero
        self.assertAlmostEqual(rs.evaluate(False), 0., delta=1e-5)
        # No derivative support yet
        self.assertRaisesUsageException(rs.evaluate, True)
        # Rotate one rigid body
        for v in (IMP.algebra.Vector3D(0,0,1),
                  IMP.algebra.Vector3D(0,1,0)):
            rot = IMP.algebra.get_rotation_about_axis(v, .15)
            tr = IMP.algebra.Transformation3D(rot)
            b1.set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
            self.assertAlmostEqual(rs.evaluate(False), 0.15, delta=1e-5)
        # Rotations about the x axis have no effect
        rot = IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0),
                                                  .15)
        tr = IMP.algebra.Transformation3D(rot)
        b1.set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
        self.assertAlmostEqual(rs.evaluate(False), 0., delta=1e-5)

    def test_show(self):
        """Test RigidBodyAnglePairScore evaluation"""
        m, p1, p2, b1, b2, ps, rs = self.make_system()
        ps.show()

if __name__ == '__main__':
    IMP.test.main()
