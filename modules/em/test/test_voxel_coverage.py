import IMP
import IMP.test
import IMP.em
import IMP.core
import IMP.algebra


class Tests(IMP.test.TestCase):

    def _make_particle(self, m, center):
        p = IMP.Particle(m)
        IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(center, 1.0))
        return p

    def test_coverage_with_gaps(self):
        """Voxels after a below-threshold voxel should still be counted"""
        dmap = IMP.em.create_density_map(10, 10, 10, 1.0)
        # Alternate columns above and below the threshold
        for z in range(10):
            for y in range(10):
                for x in range(10):
                    dmap.set_value(dmap.xyz_ind2voxel(x, y, z),
                                   1.0 if x % 2 == 0 else 0.0)
        m = IMP.Model()
        p = self._make_particle(m, IMP.algebra.Vector3D(4.5, 4.5, 4.5))
        # Smoothing radius big enough that the particle covers the whole map
        cov = IMP.em.get_percentage_of_voxels_covered_by_particles(
            dmap, [p], 20.0, IMP.algebra.get_identity_transformation_3d(),
            0.5)
        self.assertAlmostEqual(cov, 1.0, delta=1e-6)

    def test_coverage_empty_map(self):
        """Coverage of a map with no voxels above threshold should be zero"""
        dmap = IMP.em.create_density_map(5, 5, 5, 1.0)
        dmap.reset_data(0.0)
        m = IMP.Model()
        p = self._make_particle(m, IMP.algebra.Vector3D(2, 2, 2))
        cov = IMP.em.get_percentage_of_voxels_covered_by_particles(
            dmap, [p], 3.0, IMP.algebra.get_identity_transformation_3d(),
            0.5)
        self.assertEqual(cov, 0.0)


if __name__ == '__main__':
    IMP.test.main()
