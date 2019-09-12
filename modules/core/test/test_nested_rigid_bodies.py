from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import numpy as np


class GradientCalculator:

    def __init__(self, sf, pis, fks):
        self.sf = sf
        self.m = sf.get_model()
        self.pis = pis
        self.fks = fks

    def get_approximate_gradient(self, eps=1e-6):
        """Approximate gradient with central differences"""
        grad = []
        for pi, fk in zip(self.pis, self.fks):
            if not self.m.get_particle(pi).get_is_optimized(fk):
                grad.append(0.0)
                continue
            v0 = self.m.get_attribute(fk, pi)
            self.m.set_attribute(fk, pi, v0 - eps / 2)
            Sminus = self.sf.evaluate(True)
            self.m.set_attribute(fk, pi, v0 + eps / 2)
            Splus = self.sf.evaluate(True)
            self.m.set_attribute(fk, pi, v0)
            self.m.update()
            grad.append((Splus - Sminus) / eps)
        return np.array(grad)

    def get_exact_gradient(self):
        """Get IMP's computed gradient"""
        self.sf.evaluate(True)
        return np.array(
            [self.m.get_particle(pi).get_derivative(fk)
             for pi, fk in zip(self.pis, self.fks)]
        )


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
        return 0.

    def do_get_inputs(self):
        return [self.p.get_particle()]


class vonMisesFisherRestraint(IMP.Restraint):

    """Applies von Mises-Fisher distribution to quaternion."""

    def __init__(self, m, p, mu, kappa, name="vonMisesFisherRestraint %1%"):
        IMP.Restraint.__init__(self, m, name)
        self.p = p
        self.c = kappa * IMP.algebra.Vector4D(mu)

    def unprotected_evaluate(self, accum):
        if accum:
            IMP.core.RigidBody(self.p).add_to_rotational_derivatives(
                -self.c, accum)
        return -self.c * IMP.core.RigidBody(self.p).get_rotation().get_quaternion()

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
        self.assertSequenceAlmostEqual(
            rb_nested.get_torque(), rb_unnested.get_torque(),
            delta=1e-6)

    def _make_beads(self, m, nbeads, center=[0, 0, 0], radius=10):
        sphere = IMP.algebra.Sphere3D(center, radius)
        xyzs = []
        for n in range(nbeads):
            v = IMP.algebra.get_random_vector_in(sphere)
            xyz = IMP.core.XYZ.setup_particle(IMP.Particle(m), v)
            xyzs.append(xyz)
        return xyzs

    def test_all_derivatives_correct(self):
        m = IMP.Model()

        nlevels = 4
        nbeads = 10
        prob_nonrigid = 0.5
        tf = IMP.algebra.get_random_local_transformation((0, 0, 0), 10)
        rf = IMP.algebra.ReferenceFrame3D(tf)
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf)
        rb.set_coordinates_are_optimized(True)
        all_beads = []
        all_rbs = [rb]
        for level in range(nlevels):
            center = tf.get_translation()
            ps = self._make_beads(m, nbeads, center=center, radius=10)
            all_beads += ps
            for p in ps:
                if np.random.uniform() < prob_nonrigid:
                    rb.add_non_rigid_member(p)
                    nrm = IMP.core.NonRigidMember(p)
                    for k in nrm.get_internal_coordinate_keys():
                        nrm.get_particle().set_is_optimized(k, True)
                else:
                    rb.add_member(p)

            tf = IMP.algebra.get_random_local_transformation((0, 0, 0), 10)
            rf = IMP.algebra.ReferenceFrame3D(tf)
            rb_nested = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf)
            if np.random.uniform() < prob_nonrigid:
                rb.add_non_rigid_member(rb_nested)
                nrm = IMP.core.NonRigidMember(rb_nested)
                for k in nrm.get_internal_coordinate_keys() + nrm.get_internal_rotation_keys():
                    nrm.get_particle().set_is_optimized(k, True)
            else:
                rb.add_member(rb_nested)
            rb = rb_nested
            all_rbs.append(rb)

        m.update()
        ss = IMP.core.DistanceToSingletonScore(
            IMP.core.Harmonic(0, 1), (0, 0, 0)
        )
        rs = [IMP.core.SingletonRestraint(m, ss, p.get_particle_index())
              for p in all_beads]
        sf = IMP.core.RestraintsScoringFunction(rs)

        pis, fks = list(
            zip(*[(p.get_particle_index(), fk)
                  for p in all_beads + all_rbs[1:]
                  for fk in IMP.core.RigidBodyMember(p).get_internal_coordinate_keys()
                 ]
               )
        )

        gc = GradientCalculator(sf, pis, fks)
        grad_approx = gc.get_approximate_gradient(eps=1e-6)
        grad_exact = gc.get_exact_gradient()
        self.assertSequenceAlmostEqual(
            list(grad_exact), list(grad_approx), delta=1e-3
        )

        for rb in all_rbs[1:]:
            pi = rb.get_particle_index()
            nrm = IMP.core.RigidBodyMember(rb)
            fks = nrm.get_internal_rotation_keys()
            gc = GradientCalculator(sf, [pi] * 4, fks)
            grad_approx = gc.get_approximate_gradient(eps=1e-6)
            grad_exact = gc.get_exact_gradient()
            self.assertSequenceAlmostEqual(
                list(grad_exact), list(grad_approx), delta=1e-3
            )

        rb = all_rbs[0]
        pi = rb.get_particle_index()
        fks = rb.get_rotation_keys()
        gc = GradientCalculator(sf, [pi] * 4, fks)
        grad_approx = gc.get_approximate_gradient(eps=1e-6)
        grad_exact = gc.get_exact_gradient()
        self.assertSequenceAlmostEqual(
            list(grad_exact), list(grad_approx), delta=1e-3
        )

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
        self.assertSequenceAlmostEqual(
            rb_nested.get_rotational_derivatives(),
            rb_unnested.get_rotational_derivatives(),
            delta=1e-6)

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
        self.assertGreater(lderv.get_magnitude(), 0)
        self.assertAlmostEqual(
            (exp_lderv - lderv).get_magnitude(), 0, delta=1e-6)

        lcoord = IMP.core.RigidMember(point).get_internal_coordinates()
        rot_local_to_parent = IMP.core.NonRigidMember(
            nrb).get_internal_transformation().get_rotation()
        exp_lqderv = IMP.algebra.Vector4D()
        for i in range(4):
            exp_lqderv[i] = (
                lderv * rot_local_to_parent.get_gradient_of_rotated(
                    lcoord, i, False
                )
            )

        lqderv = IMP.core.NonRigidMember(
            nrb).get_internal_rotational_derivatives()
        self.assertGreater(lqderv.get_magnitude(), 0)
        self.assertAlmostEqual(
            (exp_lqderv - lqderv).get_magnitude(), 0, delta=1e-6)

    def test_identity_internal_rotation_propagates_projected_global_derivative(self):
        """Ensure propagated derivative is exact derivative projected to tangent."""
        mu = IMP.algebra.Vector4D(1, 0, 0, 0)
        kappa = 10
        rf = IMP.algebra.ReferenceFrame3D(
            IMP.algebra.get_random_local_transformation((0, 0, 0), 10))

        m = IMP.Model()
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf)
        rb_nested = IMP.core.RigidBody.setup_particle(IMP.Particle(m), rf)
        rb.add_member(rb_nested)
        rb.set_coordinates_are_optimized(True)
        rb_nested.set_coordinates_are_optimized(True)
        m.update()

        r = vonMisesFisherRestraint(m, rb_nested, mu, kappa)
        sf = IMP.core.RestraintsScoringFunction([r])
        sf.evaluate(True)

        # ensure that internal rotation is identity
        qinternal = IMP.core.RigidBodyMember(
            rb_nested).get_internal_transformation().get_rotation(
        ).get_quaternion()
        self.assertSequenceAlmostEqual(
            qinternal,
            IMP.algebra.get_identity_rotation_3d().get_quaternion(),
            delta=1e-6
        )

        # ensure that child global derivative is correct
        self.assertSequenceAlmostEqual(
            -kappa * mu,
            rb_nested.get_rotational_derivatives(),
            delta=1e-6
        )

        exp_deriv = -kappa * mu

        # ensure that derivative is propagated correctly
        self.assertSequenceAlmostEqual(
            list(rb.get_rotational_derivatives()),
            list(exp_deriv), delta=1e-6
        )


if __name__ == '__main__':
    IMP.test.main()
