import math
import IMP
import IMP.atom
import IMP.test
try:
    import jax
except ImportError:
    jax = None


def _setup_angles():
    m = IMP.Model()
    angles = []
    ps = []
    for coord in [[[0, 0, 0], [0, 0, 3], [0, 3, 0]],  # 90 degrees
                  [[0, 0, 0], [0, 0, 0], [0, 0, 0]],  # colinear
                  [[0, 0, 0], [0, 0, 3], [0, 3, 0]]]: # 90 degrees
        p0 = IMP.Particle(m)
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        pangle = IMP.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d1 = IMP.core.XYZ.setup_particle(p1)
        d2 = IMP.core.XYZ.setup_particle(p2)
        angle = IMP.atom.Angle.setup_particle(m, pangle, d0, d1, d2)
        angle.set_ideal(0.0)
        angle.set_stiffness(10.0)
        d0.set_coordinates(IMP.algebra.Vector3D(coord[0]))
        d1.set_coordinates(IMP.algebra.Vector3D(coord[1]))
        d2.set_coordinates(IMP.algebra.Vector3D(coord[2]))
        angles.append(angle)
        ps.append((p0, p1, p2))
    # Negative stiffness, should be ignored
    angles[2].set_stiffness(-1.0)
    return m, ps, angles


class Tests(IMP.test.TestCase):

    def test_angle_ss(self):
        """Test AngleSingletonScore"""
        m, _, angles = _setup_angles()
        ss = IMP.atom.AngleSingletonScore(IMP.core.Linear(0, 1))
        self.assertAlmostEqual(ss.evaluate_index(m, angles[0], None),
                               -2.5 * math.pi, delta=1e-4)
        self.assertAlmostEqual(ss.evaluate_index(m, angles[1], None),
                               -5 * math.pi, delta=1e-4)
        self.assertAlmostEqual(ss.evaluate_index(m, angles[2], None),
                               0.0, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of AngleSingletonScore"""
        import IMP.jax
        m, _, angles = _setup_angles()
        lsc = IMP.container.ListSingletonContainer(m, angles)
        ss = IMP.atom.AngleSingletonScore(IMP.core.Linear(0, 1))
        r = IMP.container.SingletonsRestraint(ss, lsc)
        imp_score = r.evaluate(False)
        ji = r._get_jax(space=IMP.jax.FreeSpace)
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        jax_score = j(jm)
        self.assertAlmostEqual(imp_score, -7.5 * math.pi, delta=0.01)
        self.assertAlmostEqual(imp_score, jax_score, delta=0.01)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multi_attr(self):
        """Test JAX AngleSingletonScore, multiple attrsets"""
        import IMP.jax
        m, ps, angles = _setup_angles()
        ss = IMP.atom.AngleSingletonScore(IMP.core.Linear(0, 1))
        r = IMP.core.SingletonRestraint(m, ss, angles[0])
        imp_score1 = r.evaluate(False)

        # Second attrset
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        IMP.core.XYZ.setup_particle(ps[0][0])
        IMP.core.XYZ.setup_particle(ps[0][1])
        IMP.core.XYZ.setup_particle(ps[0][2], (0.5, 0., 0.))
        imp_score2 = r.evaluate(False)

        jax_score = r._evaluate_jax()
        self.assertEqual(jax_score.shape, (2,))
        self.assertAlmostEqual(jax_score[0], imp_score1, delta=1e-4)
        self.assertAlmostEqual(jax_score[1], imp_score2, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_colinear_derivatives(self):
        """Test JAX AngleSingletonScore derivatives for colinear atoms"""
        import IMP.jax
        m = IMP.Model()
        ds = [IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                          IMP.algebra.Vector3D(x, 0., 0.))
              for x in (1., 0., -1.)]
        angle = IMP.atom.Angle.setup_particle(IMP.Particle(m), *ds)
        angle.set_ideal(math.pi)
        angle.set_stiffness(1.0)
        ss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0, 1))
        r = IMP.core.SingletonRestraint(m, ss, angle)
        ji = r._get_jax(space=IMP.jax.FreeSpace)
        derivs = jax.grad(ji.score_func)(ji.get_jax_model())['xyz']
        # acos-based implementations give NaN here
        for d in ds:
            for x in derivs[d.get_particle_index()]:
                self.assertAlmostEqual(float(x), 0., delta=1e-6)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_periodic(self):
        """Test JAX AngleSingletonScore with periodic boundaries"""
        import IMP.jax
        space = IMP.jax.PeriodicSpace([10., 10., 10.])
        m = IMP.Model()
        ds = [IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                          IMP.algebra.Vector3D(*c))
              for c in ((10.5, 0., 0.), (9.5, 0., 0.), (9., 1., 0.))]
        angle = IMP.atom.Angle.setup_particle(IMP.Particle(m), *ds)
        angle.set_ideal(0.5)
        angle.set_stiffness(2.0)
        ss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0, 1))
        r = IMP.core.SingletonRestraint(m, ss, angle)
        imp_score = r.evaluate(False)
        # Put each atom in the periodic cell, splitting the angle across
        # the boundary; the score should not change
        ds[0].set_coordinates(IMP.algebra.Vector3D(0.5, 0., 0.))
        ji = r._get_jax(space)
        jax_score = jax.jit(ji.score_func)(ji.get_jax_model())
        self.assertAlmostEqual(jax_score, imp_score, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
