from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import numpy as np


class DummyRestraint(IMP.Restraint):

    """Adds random derivatives to particles."""

    def __init__(self, m, ps1, ps2, name="DummyRestraint %1%"):
        IMP.Restraint.__init__(self, m, name)
        self.ps1 = ps1
        self.ps2 = ps2

    def unprotected_evaluate(self, accum):
        if accum:
            for p1, p2 in zip(self.ps1, self.ps2):
                v = np.random.normal(size=3)
                IMP.core.XYZ(p1).add_to_derivatives(
                    v, accum)
                IMP.core.XYZ(p2).add_to_derivatives(
                    v, accum)
        return 0.

    def do_get_inputs(self):
        return IMP.get_particles(self.get_model(), self.ps1 + self.ps2)


class DummyRestraint2(IMP.Restraint):

    """Adds random derivatives to particles."""

    def __init__(self, m, p, name="DummyRestraint2 %1%"):
        IMP.Restraint.__init__(self, m, name)
        self.p = p

    def unprotected_evaluate(self, accum):
        if accum:
            v = np.random.normal(size=3)
            IMP.core.XYZ(self.p).add_to_derivatives(v, accum)
            # q = np.random.normal(size=4)
            # IMP.core.RigidBody(self.p).add_to_rotational_derivatives(q, accum)
        return 0.

    def do_get_inputs(self):
        return [self.p.get_particle()]


class Tests(IMP.test.TestCase):

    """Tests for RigidBody function"""

    def test_nested(self):
        """Test nested rigid bodies and close pairs"""
        m = IMP.Model()
        ps = [m.add_particle("p") for i in range(0, 10)]
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 10, 10))
        ds = [IMP.core.XYZR.setup_particle(
            m, p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb),
                                    1)) for p in ps]

        rb0 = IMP.core.RigidBody.setup_particle(m, m.add_particle("rb0"),
                                                ps[0:3])
        rb1 = IMP.core.RigidBody.setup_particle(m, m.add_particle("rb1"),
                                                ps[3:6])
        rb2 = IMP.core.RigidBody.setup_particle(m, m.add_particle("rb2"),
                                                ps[6:10])
        rb0.add_member(rb1)
        cpf = IMP.core.RigidClosePairsFinder()
        cpf.set_distance(.5)
        # check that no internal checks fail
        cp0 = cpf.get_close_pairs(m, ps)
        cp1 = cpf.get_close_pairs(m, ps[3:])

    def test_torque_from_nested_equal_to_unnested(self):
        """Test nested rigid bodies produce same torque as unnested."""
        sphere1 = IMP.algebra.Sphere3D([0, 0, 0], 5)
        sphere2 = IMP.algebra.Sphere3D([0, 0, 10], 10)
        N1, N2 = 20, 20
        vs1 = [
            IMP.algebra.get_random_vector_in(sphere1) for n in range(N1)]
        vs2 = [
            IMP.algebra.get_random_vector_in(sphere2) for n in range(N2)]

        m = IMP.Model()
        ps_unnested = []
        rb_ps1, rb_ps2 = [], []
        for v in vs1:
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            ps_unnested.append(xyz)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            rb_ps1.append(xyz)

        for v in vs2:
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            ps_unnested.append(xyz)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            rb_ps2.append(xyz)

        ps_nested = rb_ps1 + rb_ps2

        rb_unnested = IMP.core.RigidBody.setup_particle(
            IMP.Particle(m), ps_unnested)
        rb_member = IMP.core.RigidBody.setup_particle(
            IMP.Particle(m), rb_ps2)
        rb_nested = IMP.core.RigidBody.setup_particle(
            IMP.Particle(m), rb_unnested.get_reference_frame())
        for p in rb_ps1 + [rb_member]:
            rb_nested.add_member(p)
        m.update()

        r = DummyRestraint(m, ps_unnested, ps_nested)
        sf = IMP.core.RestraintsScoringFunction([r])
        sf.evaluate(True)

        self.assertGreater(
            rb_unnested.get_torque().get_magnitude(), 0)
        np.testing.assert_allclose(
            rb_nested.get_torque(), rb_unnested.get_torque(),
            rtol=1e-6)

    def test_quaternion_derivatives_from_nested_equal_to_unnested(self):
        """Test nested rigid bodies have same quaternion derivatives as unnested."""
        sphere1 = IMP.algebra.Sphere3D([0, 0, 0], 5)
        sphere2 = IMP.algebra.Sphere3D([0, 0, 10], 10)
        N1, N2 = 20, 20
        vs1 = [
            IMP.algebra.get_random_vector_in(sphere1) for n in range(N1)]
        vs2 = [
            IMP.algebra.get_random_vector_in(sphere2) for n in range(N2)]

        m = IMP.Model()
        ps_unnested = []
        rb_ps1, rb_ps2 = [], []
        for v in vs1:
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            ps_unnested.append(xyz)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            rb_ps1.append(xyz)

        for v in vs2:
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            ps_unnested.append(xyz)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            rb_ps2.append(xyz)

        ps_nested = rb_ps1 + rb_ps2

        rb_unnested = IMP.core.RigidBody.setup_particle(
            IMP.Particle(m), ps_unnested)
        rb_member = IMP.core.RigidBody.setup_particle(
            IMP.Particle(m), rb_ps2)
        rb_nested = IMP.core.RigidBody.setup_particle(
            IMP.Particle(m), rb_unnested.get_reference_frame())
        for p in rb_ps1 + [rb_member]:
            rb_nested.add_member(p)
        rb_unnested.set_coordinates_are_optimized(True)
        rb_nested.set_coordinates_are_optimized(True)
        m.update()

        r = DummyRestraint(m, ps_unnested, ps_nested)
        sf = IMP.core.RestraintsScoringFunction([r])
        sf.evaluate(True)

        self.assertGreater(
            rb_unnested.get_rotational_derivatives().get_magnitude(), 0)
        np.testing.assert_allclose(
            rb_nested.get_rotational_derivatives(),
            rb_unnested.get_rotational_derivatives(),
            rtol=1e-6)

    def test_non_rigid_member_local_derivatives_are_correct(self):
        """Test global derivatives on non-rigid member propagated to local."""

        rf1 = IMP.algebra.ReferenceFrame3D(
            IMP.algebra.get_random_local_transformation((0,0,0), 10))
        rf2 = IMP.algebra.ReferenceFrame3D(
            IMP.algebra.get_random_local_transformation((0,0,0), 10))

        m = IMP.Model()
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf1)
        nrb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf2)
        point = IMP.core.XYZ.setup_particle(IMP.Particle(m), (1, 2, 3))
        nrb.add_member(point)
        rb.add_non_rigid_member(nrb)
        m.update()

        self.assertTrue(IMP.core.NonRigidMember.get_is_setup(nrb))
        self.assertTrue(IMP.core.RigidBodyMember.get_is_setup(nrb))
        self.assertTrue(IMP.core.RigidBody.get_is_setup(nrb))
        self.assertFalse(IMP.core.RigidMember.get_is_setup(nrb))

        r = DummyRestraint2(m, point)
        sf = IMP.core.RestraintsScoringFunction([r])
        sf.evaluate(True)

        rot_global_to_parent = rb.get_reference_frame(
            ).get_transformation_to().get_rotation().get_inverse()
        exp_lderv = rot_global_to_parent * point.get_derivatives()
        lderv = IMP.core.NonRigidMember(nrb).get_internal_derivatives()
        self.assertAlmostEqual(
            (exp_lderv - lderv).get_magnitude(), 0, delta=1e-6)

        lcoord = IMP.core.RigidMember(point).get_internal_coordinates()
        rot_local_to_parent = IMP.core.NonRigidMember(
            nrb).get_internal_transformation().get_rotation()
        exp_lqderv = IMP.algebra.Vector4D()
        for i in range(4):
            exp_lqderv[i] = (
                lderv * rot_local_to_parent.get_derivative(lcoord, i))

        lqderv = IMP.core.NonRigidMember(
            nrb).get_internal_rotational_derivatives()
        self.assertAlmostEqual(
            (exp_lqderv - lqderv).get_magnitude(), 0, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
