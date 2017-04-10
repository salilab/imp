import random

import IMP
import IMP.core
import IMP.algebra
import IMP.test


def _harmonic(x, mean, k):
    return .5 * k * (x - mean)**2


def _harmonic_lb(x, mean, k):
    diff = x - mean
    if diff > 0:
        return 0
    return _harmonic(x, mean, k)


class Tests(IMP.test.TestCase):

    """Test for PairScores on surface distances"""

    def test_init(self):
        """Test pair scores initialize without error."""
        h = IMP.core.Harmonic(0, 1)
        ps = IMP.core.SurfaceDistancePairScore(h)
        ps = IMP.core.SurfaceHeightPairScore(h)
        ps = IMP.core.SurfaceDepthPairScore(h)
        ps = IMP.core.HarmonicSurfaceDistancePairScore(0, 1)
        ps = IMP.core.HarmonicSurfaceHeightPairScore(0, 1)
        ps = IMP.core.HarmonicSurfaceDepthPairScore(0, 1)
        ps = IMP.core.SoftSubSurfacePairScore(1.)
        ps = IMP.core.SoftSuperSurfacePairScore(1.)

    def test_dist_values(self):
        """Test harmonic and generic distance scores."""
        m = IMP.Model()
        s = IMP.core.Surface.setup_particle(IMP.Particle(m))
        d = IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                         IMP.algebra.Sphere3D((0, 0, 0), 1.0))
        pip = (s.get_particle_index(), d.get_particle_index())
        da = IMP.DerivativeAccumulator()
        h = IMP.core.Harmonic(0, 1)
        ps1 = IMP.core.SurfaceDistancePairScore(h)
        for i in range(100):
            k = random.uniform(0, 10)
            mean = random.uniform(-5, 5)
            h.set_k(k)
            h.set_mean(mean)
            d.set_coordinates(
                30 * IMP.algebra.get_random_vector_on_unit_sphere())
            dist = IMP.core.get_distance(s, d)
            exp_score = _harmonic(dist, mean, k)
            self.assertAlmostEqual(ps1.evaluate_index(m, pip, da), exp_score)
            ps2 = IMP.core.HarmonicSurfaceDistancePairScore(mean, k)
            self.assertAlmostEqual(ps2.evaluate_index(m, pip, da), exp_score)
            ps2.set_was_used(True)

    def test_height_values(self):
        """Test harmonic and generic height scores."""
        m = IMP.Model()
        s = IMP.core.Surface.setup_particle(IMP.Particle(m))
        d = IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                         IMP.algebra.Sphere3D((0, 0, 0), 1.0))
        pip = (s.get_particle_index(), d.get_particle_index())
        da = IMP.DerivativeAccumulator()
        h = IMP.core.Harmonic(0, 1)
        ps1 = IMP.core.SurfaceHeightPairScore(h)
        for i in range(100):
            k = random.uniform(0, 10)
            mean = random.uniform(-5, 5)
            h.set_k(k)
            h.set_mean(mean)
            d.set_coordinates(
                30 * IMP.algebra.get_random_vector_on_unit_sphere())
            dist = IMP.core.get_height(s, d)
            exp_score = _harmonic(dist, mean, k)
            self.assertAlmostEqual(ps1.evaluate_index(m, pip, da), exp_score)
            ps2 = IMP.core.HarmonicSurfaceHeightPairScore(mean, k)
            self.assertAlmostEqual(ps2.evaluate_index(m, pip, da), exp_score)
            ps2.set_was_used(True)

    def test_depth_values(self):
        """Test harmonic and generic distance scores."""
        m = IMP.Model()
        s = IMP.core.Surface.setup_particle(IMP.Particle(m))
        d = IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                         IMP.algebra.Sphere3D((0, 0, 0), 1.0))
        pip = (s.get_particle_index(), d.get_particle_index())
        da = IMP.DerivativeAccumulator()
        h = IMP.core.Harmonic(0, 1)
        ps1 = IMP.core.SurfaceDepthPairScore(h)
        for i in range(100):
            k = random.uniform(0, 10)
            mean = random.uniform(-5, 5)
            h.set_k(k)
            h.set_mean(mean)
            d.set_coordinates(
                30 * IMP.algebra.get_random_vector_on_unit_sphere())
            dist = IMP.core.get_depth(s, d)
            exp_score = _harmonic(dist, mean, k)
            self.assertAlmostEqual(ps1.evaluate_index(m, pip, da), exp_score)
            ps2 = IMP.core.HarmonicSurfaceDepthPairScore(mean, k)
            self.assertAlmostEqual(ps2.evaluate_index(m, pip, da), exp_score)
            ps2.set_was_used(True)

    def test_soft_restraint_values(self):
        """Test soft sub- and super-surface restraints."""
        m = IMP.Model()
        s = IMP.core.Surface.setup_particle(IMP.Particle(m))
        d = IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                         IMP.algebra.Sphere3D((0, 0, 0), 1.0))
        pip = (s.get_particle_index(), d.get_particle_index())
        da = IMP.DerivativeAccumulator()
        ps_sup = IMP.core.SoftSuperSurfacePairScore(1.)
        ps_sub = IMP.core.SoftSubSurfacePairScore(1.)
        for i in range(100):
            d.set_coordinates(
                30 * IMP.algebra.get_random_vector_on_unit_sphere())
            height = IMP.core.get_height(s, d)
            depth = IMP.core.get_depth(s, d)
            exp_super_score = _harmonic_lb(height, 0., 1.)
            exp_sub_score = _harmonic_lb(depth, 0., 1.)
            self.assertAlmostEqual(ps_sup.evaluate_index(m, pip, da),
                                   exp_super_score)
            self.assertAlmostEqual(ps_sub.evaluate_index(m, pip, da),
                                   exp_sub_score)


if __name__ == '__main__':
    IMP.test.main()
