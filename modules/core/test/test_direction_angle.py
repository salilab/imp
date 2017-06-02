import math

import IMP
import IMP.algebra
import IMP.core
import IMP.atom
import IMP.test


class AngleRestraint(IMP.Restraint):

    """Harmonically restrain an angle between two directions to a value."""

    def __init__(self, m, pi, a, k):
        super(AngleRestraint, self).__init__(m, "AngleRestraint%1%")
        self.pi = pi
        self.a = a
        self.k = k

    def do_add_score_and_derivatives(self, sa):
        a = IMP.core.DirectionAngle(self.get_model(), self.pi)
        angle = a.get_angle()
        diff = angle - self.a
        score = .5 * self.k * diff**2
        if sa.get_derivative_accumulator():
            derv = self.k * diff
            a.add_to_angle_derivative(derv, sa.get_derivative_accumulator())
        sa.add_score(score)

    def do_get_inputs(self):
        return [self.get_model().get_particle(self.pi)]


class Tests(IMP.test.TestCase):

    """Tests for DirectionAngle."""

    def _create_directions(self, m, v1=IMP.algebra.Vector3D(0, 0, 1),
                           v2=IMP.algebra.Vector3D(0, 0, 1)):
        d1 = IMP.core.Direction.setup_particle(IMP.Particle(m), v1)
        d2 = IMP.core.Direction.setup_particle(IMP.Particle(m), v2)
        return (d1, d2)

    def test_init(self):
        """Test basic set-up."""
        m = IMP.Model()
        ds = self._create_directions(m)
        a = IMP.core.DirectionAngle.setup_particle(IMP.Particle(m), ds)
        self.assertTrue(IMP.core.DirectionAngle.get_is_setup(
            m, a.get_particle_index()))
        self.assertEqual(a.get_particle(0), ds[0].get_particle())
        self.assertEqual(a.get_particle(1), ds[1].get_particle())
        a = IMP.core.DirectionAngle.setup_particle(IMP.Particle(m), ds[0],
                                                   ds[1])
        self.assertTrue(IMP.core.DirectionAngle.get_is_setup(
            m, a.get_particle_index()))
        self.assertEqual(a.get_particle(0), ds[0].get_particle())
        self.assertEqual(a.get_particle(1), ds[1].get_particle())
        a = IMP.core.DirectionAngle.setup_particle(
            IMP.Particle(m), ds[0].get_particle_index(),
            ds[1].get_particle_index())
        self.assertTrue(IMP.core.DirectionAngle.get_is_setup(
            m, a.get_particle_index()))
        self.assertEqual(a.get_particle(0), ds[0].get_particle())
        self.assertEqual(a.get_particle(1), ds[1].get_particle())

    def test_angle_updated(self):
        """Test angle is updated correctly when direction moves."""
        m = IMP.Model()
        ds = self._create_directions(m, v1=IMP.algebra.Vector3D(0, 0, 1))
        a = IMP.core.DirectionAngle.setup_particle(IMP.Particle(m), ds)
        for i in range(0, 181, 5):
            angle = i * math.pi / 180.
            rot = IMP.algebra.get_rotation_about_axis(
                IMP.algebra.Vector3D(0, 1, 0), angle)
            ds[1].set_direction(rot.get_rotated(ds[0].get_direction()))
            m.update()
            self.assertAlmostEqual(a.get_angle(), angle, delta=1e-6)

    def test_derivatives_updated(self):
        """Test derivative is updated correctly after score evaluation."""
        m = IMP.Model()
        ds = self._create_directions(m)
        a = IMP.core.DirectionAngle.setup_particle(IMP.Particle(m), ds)
        a0 = math.pi / 2.
        angle = math.pi / 4.
        r = AngleRestraint(m, a.get_particle_index(), a0, 1.)
        rot = IMP.algebra.get_rotation_about_axis((0, 1, 0), angle)
        ds[1].set_direction(rot.get_rotated(ds[0].get_direction()))
        r.evaluate(True)
        exp_derv = 2**.5 * math.pi / 4. * ds[1].get_direction()
        exp_derv -= (exp_derv * ds[0].get_direction()) * ds[0].get_direction()
        self.assertAlmostEqual(
            (exp_derv - ds[0].get_direction_derivatives()).get_magnitude(),
            0., delta=1e-6)

    def test_optimization(self):
        """Test optimize angle between two directions."""
        v = IMP.algebra.Vector3D(0, 0, 1)
        angles = [0.25 * math.pi, 0.3 * math.pi, 0.6 * math.pi, 0.75 * math.pi]
        for angle in angles:
            m = IMP.Model()
            ds = self._create_directions(m, v1=v)
            a = IMP.core.DirectionAngle.setup_particle(IMP.Particle(m), ds)
            r = AngleRestraint(m, a.get_particle_index(), math.pi / 2., 10.)
            rot = IMP.algebra.get_rotation_about_axis((0, 1, 0), angle)
            ds[1].set_direction(rot.get_rotated(ds[0].get_direction()))
            ds[1].set_direction_is_optimized(True)
            sf = IMP.core.RestraintsScoringFunction([r])
            self.assertAlmostEqual(math.acos(ds[1].get_direction() * v), angle,
                                   delta=1e-6)
            self.assertGreater(sf.evaluate(False), .4)
            opt = IMP.core.ConjugateGradients(m)
            opt.set_scoring_function(sf)
            self.assertLess(opt.optimize(50), 1e-6)
            self.assertAlmostEqual(math.acos(ds[1].get_direction() * v),
                                   math.pi / 2., delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
