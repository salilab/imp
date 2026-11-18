import IMP
import IMP.test
import IMP.em
import IMP.core
import IMP.atom
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_score_of_own_map(self):
        """compute_fitting_score of particles in their own map is ~zero"""
        m = IMP.Model()
        ps = []
        for x, y, z in ((9., 5., 5.), (12., 9., 4.), (4., 5., 5.),
                        (8., 8., 8.)):
            p = IMP.Particle(m)
            IMP.core.XYZR.setup_particle(
                p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x, y, z), 1.0))
            IMP.atom.Mass.setup_particle(p, 1.0)
            ps.append(p)
        # This map has zero density around its border
        em_map = IMP.em.SampledDensityMap(ps, 3.0, 1.0,
                                          IMP.atom.Mass.get_mass_key())
        em_map.resample()
        score = IMP.em.compute_fitting_score(ps, em_map)
        self.assertAlmostEqual(score, 0.0, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
