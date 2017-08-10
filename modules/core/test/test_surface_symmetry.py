import math
import IMP
import IMP.core
import IMP.algebra
import IMP.test


def _get_random_reference_frame():
    return IMP.algebra.ReferenceFrame3D(
        IMP.algebra.get_random_local_transformation((0, 0, 0), 10., 6.))


def _get_angle_harmonic_normal_score(s, d, k):
    norm = s.get_normal()
    v = (d.get_coordinates() - s.get_coordinates()).get_unit_vector()
    angle = math.acos(v * norm)
    if angle == 0:
        return 0, -k * v
    score = .5 * k * angle**2
    derv = -k * angle * v / math.sin(angle)
    return score, derv


class NormalRestraint(IMP.Restraint):

    """Restrain surface normal to vector from surface center to sphere."""

    def __init__(self, m, s, d, k=1):
        IMP.Restraint.__init__(self, m, "NormalRestraint%1%")
        self.s, self.d = s, d
        self.k = k

    def do_add_score_and_derivatives(self, sa):
        s = IMP.core.Surface(self.get_model(), self.s)
        d = IMP.core.XYZR(self.get_model(), self.d)

        score, derv = _get_angle_harmonic_normal_score(s, d, self.k)
        if sa.get_derivative_accumulator():
            s.add_to_normal_derivatives(derv, sa.get_derivative_accumulator())
        sa.add_score(score)

    def do_get_inputs(self):
        return [self.get_model().get_particle(self.s),
                self.get_model().get_particle(self.d)]


class Tests(IMP.test.TestCase):

    """Tests for IMP.core.SurfaceSymmetryConstraint"""

    def test_init(self):
        """Test initialization and setup."""
        m = IMP.Model()
        rb1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                IMP.algebra.ReferenceFrame3D())
        rb2 = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                IMP.algebra.ReferenceFrame3D())
        s1 = IMP.core.Surface.setup_particle(IMP.Particle(m))
        s2 = IMP.core.Surface.setup_particle(IMP.Particle(m))
        c = IMP.core.SurfaceSymmetryConstraint(m, (s1, s2), (rb1, rb2))
        m.add_score_state(c)
        m.update()

    def test_constraint_enforced(self):
        """Test first surface is moved to match relative orientation."""
        m = IMP.Model()
        rb1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                IMP.algebra.ReferenceFrame3D())
        rb2 = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                IMP.algebra.ReferenceFrame3D())
        s1 = IMP.core.Surface.setup_particle(IMP.Particle(m))
        s2 = IMP.core.Surface.setup_particle(IMP.Particle(m))
        c = IMP.core.SurfaceSymmetryConstraint(m, (s1, s2), (rb1, rb2))
        m.add_score_state(c)

        for i in range(100):
            s2.set_reference_frame(_get_random_reference_frame())
            rb1.set_reference_frame(_get_random_reference_frame())
            rb2.set_reference_frame(_get_random_reference_frame())
            m.update()

            rb_tf = IMP.algebra.get_transformation_from_first_to_second(
                rb2.get_reference_frame(), rb1.get_reference_frame())

            self.assertAlmostEqual(
                (rb_tf.get_rotation().get_rotated(s2.get_normal()) -
                 s1.get_normal()).get_magnitude(), 0.)
            self.assertAlmostEqual(
                (rb_tf.get_transformed(s2.get_coordinates()) -
                 s1.get_coordinates()).get_magnitude(), 0.)

    def test_derivatives_updated(self):
        """Test constraint correctly rotates derivatives."""
        m = IMP.Model()
        rb1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                IMP.algebra.ReferenceFrame3D())
        rb2 = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                IMP.algebra.ReferenceFrame3D())
        IMP.core.XYZR.setup_particle(rb1, 1)
        IMP.core.XYZR.setup_particle(rb2, 1)
        s1 = IMP.core.Surface.setup_particle(IMP.Particle(m))
        s2 = IMP.core.Surface.setup_particle(IMP.Particle(m))
        c = IMP.core.SurfaceSymmetryConstraint(m, (s1, s2), (rb1, rb2))
        sc = IMP.core.HarmonicSurfaceDistancePairScore(0, 1)
        r1 = IMP.core.PairRestraint(m, sc, (s1.get_particle_index(),
                                            rb1.get_particle_index()))
        r2 = NormalRestraint(m, s1, rb1, 1.)
        m.add_score_state(c)

        tf2 = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(0, 0, 1))
        rb2.set_reference_frame(IMP.algebra.ReferenceFrame3D(tf2))
        s2.set_normal(IMP.algebra.Vector3D(1, 0, 0))

        for i in range(100):
            s1.set_reference_frame(_get_random_reference_frame())
            rb1.set_reference_frame(_get_random_reference_frame())

            rb_tf = IMP.algebra.get_transformation_from_first_to_second(
                rb1.get_reference_frame(), rb2.get_reference_frame())

            r1.evaluate(True)
            r2.evaluate(True)

            untrans_derv = s1.get_derivatives()
            trans_derv = rb_tf.get_rotation().get_rotated(untrans_derv)
            self.assertAlmostEqual(
                (trans_derv - s2.get_derivatives()).get_magnitude(), 0.)

            untrans_derv = s1.get_normal_derivatives()
            trans_derv = rb_tf.get_rotation().get_rotated(untrans_derv)

            self.assertAlmostEqual(
                (trans_derv - s2.get_normal_derivatives()).get_magnitude(), 0.,
                delta=1e-6)


if __name__ == "__main__":
    IMP.test.main()
