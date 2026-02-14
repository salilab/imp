import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import random
import contextlib
try:
    import jax
except ImportError:
    jax = None


@contextlib.contextmanager
def internal_checks_disabled():
    """Temporarily disable internal checks"""
    _old_check_level = IMP.get_check_level()
    IMP.set_check_level(IMP.USAGE)
    yield
    IMP.set_check_level(_old_check_level)


class Tests(IMP.test.TestCase):

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

        # Bond with no length
        b.set_length(-1.0)
        self.assertEqual(ss.evaluate_index(m, b, None), 0)

        # Bond with negative stiffness (will be treated as 1)
        b.set_length(3.0)
        b.set_stiffness(-1000)
        self.assertEqual(ss.evaluate_index(m, b, None), -1)

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
        bonds = []
        # Include bond with negative length (should score zero) and
        # one with negative stiffness (should act as if stiffness=1.0)
        for length, stiffness in ((1.0, 20.0), (-1.0, 20.0), (1.0, -20.0)):
            p0 = IMP.Particle(m)
            p1 = IMP.Particle(m)
            d0 = IMP.core.XYZ.setup_particle(p0)
            d1 = IMP.core.XYZ.setup_particle(p1)
            b0 = IMP.atom.Bonded.setup_particle(p0)
            b1 = IMP.atom.Bonded.setup_particle(p1)
            # Force IMP to allow creating a bond with negative length
            # by disabling internal checks
            with internal_checks_disabled():
                b = IMP.atom.create_custom_bond(b0, b1, length, stiffness)
            bonds.append(b)
        lsc = IMP.container.ListSingletonContainer(m, bonds)
        ss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0.1, 1.2))
        r = IMP.container.SingletonsRestraint(ss, lsc)
        imp_score = r.evaluate(False)
        ji = r._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        jax_score = j(jm)
        self.assertAlmostEqual(imp_score, 243.132, delta=0.01)
        self.assertAlmostEqual(imp_score, jax_score, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
