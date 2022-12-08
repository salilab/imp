import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):

    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        # init IMP model ( the environment)
        self.mdl = IMP.Model()
        self.mh = IMP.atom.read_pdb(
            self.get_input_file_name("1z5s_A.pdb"),
            self.mdl, IMP.atom.NonWaterNonHydrogenPDBSelector())
        self.particles = IMP.core.get_leaves(self.mh)

    def test_sample_map(self):
        """Check that protein surface sampling works"""
        voxel_size = 1.
        model_map = IMP.em.SurfaceShellDensityMap(self.particles, voxel_size)
        for p in self.particles:
            self.assertGreater(
                model_map.get_value(
                    IMP.core.XYZ(p.get_particle()).get_coordinates()),
                0.99,
                "map was not sampled correctly")


if __name__ == '__main__':
    IMP.test.main()
