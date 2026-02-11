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
    # Negative stiffness, should be ignored
    angles[2].set_stiffness(-1.0)
    return m, angles


class Tests(IMP.test.TestCase):

    def test_angle_ss(self):
        """Test AngleSingletonScore"""
        m, angles = _setup_angles()
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
        m, angles = _setup_angles()
        lsc = IMP.container.ListSingletonContainer(m, angles)
        ss = IMP.atom.AngleSingletonScore(IMP.core.Linear(0, 1))
        r = IMP.container.SingletonsRestraint(ss, lsc)
        imp_score = r.evaluate(False)
        ji = r._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        jax_score = j(jm)
        self.assertAlmostEqual(imp_score, -7.5 * math.pi, delta=0.01)
        self.assertAlmostEqual(imp_score, jax_score, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
