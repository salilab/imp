import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import random
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    """Test the symmetry restraint"""

    def test_bd_ss(self):
        """Test the bond decorator score"""
        m = IMP.Model()
        p0 = IMP.Particle(m)
        p1 = IMP.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d1 = IMP.core.XYZ.setup_particle(p1)
        b0 = IMP.atom.Bonded.setup_particle(p0)
        b1 = IMP.atom.Bonded.setup_particle(p1)
        b = IMP.atom.create_custom_bond(b0, b1, 3, 2)
        ss = IMP.atom.BondSingletonScore(IMP.core.Linear(0, 1))

        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        d1.set_coordinates(IMP.algebra.Vector3D(0, 0, 3))
        self.assertEqual(ss.evaluate_index(m, b, None), 0)

        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        d1.set_coordinates(IMP.algebra.Vector3D(0, 0, 4))
        self.assertEqual(ss.evaluate_index(m, b, None), 2)

        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        d1.set_coordinates(IMP.algebra.Vector3D(0, 0, 2))
        self.assertEqual(ss.evaluate_index(m, b, None), -2)

    def test_bd_deriv(self):
        """Test derivatives of the bond SingletonScore"""
        m = IMP.Model()
        p0 = IMP.Particle(m)
        p1 = IMP.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d1 = IMP.core.XYZ.setup_particle(p1)
        b0 = IMP.atom.Bonded.setup_particle(p0)
        b1 = IMP.atom.Bonded.setup_particle(p1)
        b = IMP.atom.create_custom_bond(b0, b1, 1.0, 20.0)
        ss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
        r = IMP.core.SingletonRestraint(m, ss, b)
        sf = IMP.core.RestraintsScoringFunction([r])

        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        d1.set_coordinates(
            IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
            * (random.random() * 4.0 + 2.0))

        self.assertXYZDerivativesInTolerance(sf, d0, 2.0, 5.0)
        self.assertXYZDerivativesInTolerance(sf, d1, 2.0, 5.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of BondSingletonScore"""
        m = IMP.Model()
        p0 = IMP.Particle(m)
        p1 = IMP.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d1 = IMP.core.XYZ.setup_particle(p1)
        b0 = IMP.atom.Bonded.setup_particle(p0)
        b1 = IMP.atom.Bonded.setup_particle(p1)
        b = IMP.atom.create_custom_bond(b0, b1, 1.0, 20.0)
        ss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0.1, 1.2))
        r = IMP.core.SingletonRestraint(m, ss, b)
        imp_score = r.evaluate(False)
        ji = r._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        jax_score = j(jm)
        self.assertAlmostEqual(imp_score, 242.406, delta=0.01)
        self.assertAlmostEqual(imp_score, jax_score, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
