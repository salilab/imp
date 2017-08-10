import math

import IMP
import IMP.algebra
import IMP.core
import IMP.atom
import IMP.test


class DirectionRestraint(IMP.Restraint):

    """Harmonically restrain a Direction to a vector."""

    def __init__(self, m, pi, v, k):
        super(DirectionRestraint, self).__init__(m, "DirectionRestraint%1%")
        self.pi, self.u = pi, v.get_unit_vector()
        self.uf = IMP.core.Cosine(k, 1, 0)

    def do_add_score_and_derivatives(self, sa):
        d = IMP.core.Direction(self.get_model(), self.pi)
        angle = math.acos(d.get_direction() * self.u)
        self.uf.evaluate(angle)
        if sa.get_derivative_accumulator():
            score, derv = self.uf.evaluate_with_derivative(angle)
            if angle == 0:
                derv = IMP.algebra.Vector3D(0, 0, 0)
            else:
                derv = derv * -self.u / math.sin(angle)
            d.add_to_direction_derivatives(derv,
                                           sa.get_derivative_accumulator())
        else:
            score = self.uf.evaluate(angle)
        sa.add_score(score)

    def do_get_inputs(self):
        return [self.get_model().get_particle(self.pi)]


class Tests(IMP.test.TestCase):

    """Tests for Direction."""

    def test_init(self):
        """Test basic Direction set-up."""
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.Direction.setup_particle(
                p, IMP.algebra.Vector3D(0, 0, 1))
        self.assertTrue(IMP.core.Direction.get_is_setup(m, p.get_index()))
        self.assertAlmostEqual((IMP.algebra.Vector3D(0, 0, 1) -
                                d.get_direction()).get_magnitude(), 0.)

    def test_init_with_non_unit_vector(self):
        """Test Direction set-up with vector of non-unit length."""
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.Direction.setup_particle(
                p, IMP.algebra.Vector3D(0, 0, 10))
        self.assertTrue(IMP.core.Direction.get_is_setup(m, p.get_index()))
        self.assertAlmostEqual((IMP.algebra.Vector3D(0, 0, 1) -
                                d.get_direction()).get_magnitude(), 0.)
        p = IMP.Particle(m)
        d = IMP.core.Direction.setup_particle(
                p, IMP.algebra.Vector3D(0, 0, 0))
        self.assertTrue(IMP.core.Direction.get_is_setup(m, p.get_index()))
        self.assertAlmostEqual(d.get_direction().get_magnitude(), 1.)

    def test_reflect(self):
        """Test direction is reversed when reflected."""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d.reflect()
        self.assertAlmostEqual((d.get_direction() -
                                IMP.algebra.Vector3D(0, 0, -1)
                                ).get_magnitude(), 0.)

    def test_direction_unit_length_enforced(self):
        """Test direction is unit length after model update."""
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.Direction.setup_particle(
                p, IMP.algebra.Vector3D(0, 0, 1))
        m.set_attribute(d.get_direction_key(0), p.get_index(), 0)
        m.set_attribute(d.get_direction_key(1), p.get_index(), 10)
        m.set_attribute(d.get_direction_key(2), p.get_index(), 0)
        m.update()
        self.assertAlmostEqual((IMP.algebra.Vector3D(0, 1, 0) -
                                d.get_direction()).get_magnitude(), 0.)

    def test_direction_tangent_enforced(self):
        """Test derivative is tangent after score evaluate."""
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.Direction.setup_particle(
                p, IMP.algebra.Vector3D(0, 0, 1))
        r = DirectionRestraint(m, p.get_index(),
                               IMP.algebra.Vector3D(1, 1, 1), 10.)
        r.evaluate(True)
        exp_derv = -10 * IMP.algebra.Vector3D(1, 1, 0) / 3**.5
        self.assertAlmostEqual(
            (exp_derv -
             d.get_direction_derivatives()).get_magnitude(), 0., delta=1e-6)

    def test_optimization(self):
        """Test optimize angle between direction and vector."""
        v = IMP.algebra.Vector3D(0, 0, 1)
        axis = IMP.algebra.Vector3D(0, 1, 0)
        angles = [0.25 * math.pi, 0.3 * math.pi, 0.6 * math.pi, 0.75 * math.pi]
        for angle in angles:
            m = IMP.Model()
            rot = IMP.algebra.get_rotation_about_axis(axis, angle)
            d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                                  rot.get_rotated(v))
            d.set_direction_is_optimized(True)
            r = DirectionRestraint(m, d.get_particle_index(), v, 10.)
            sf = IMP.core.RestraintsScoringFunction([r])
            self.assertAlmostEqual(math.acos(d.get_direction() * v), angle,
                                   delta=1e-6)
            self.assertGreater(sf.evaluate(False), 1.)
            opt = IMP.core.ConjugateGradients(m)
            opt.set_scoring_function(sf)
            self.assertLess(opt.optimize(50), 1e-6)
            self.assertAlmostEqual(math.acos(d.get_direction() * v), 0.,
                                   delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
