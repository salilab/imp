import IMP
import IMP.em
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    """Class to testing conversion from em to particles"""

    def load_density_map(self):
        # Note: This test used to work with in.em, we moved to mrc
        # as it was not clear how to save origin in em format.
        # Resolve with Frido.
        self.mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(
            self.get_input_file_name("in.mrc"),
            self.mrw)
        self.scene.set_was_used(True)
        self.scene_threshold = 0.01
        self.scene_centroid = self.scene.get_centroid()

    def test_convert_to_particles(self):
        self.load_density_map()
        mdl = IMP.Model()
        ps = IMP.em.density2particles(self.scene, self.scene_threshold, mdl)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(IMP.core.get_centroid(ps),
                                     self.scene_centroid),
            0,
            delta=0.1)

    def test_convert_to_vectors(self):
        self.load_density_map()
        vecs = IMP.em.density2vectors(self.scene, self.scene_threshold)
        vecs_mean = IMP.algebra.Vector3D(0., 0., 0.)
        for v in vecs:
            vecs_mean = vecs_mean + v
        vecs_mean = vecs_mean / len(vecs)
        self.assertAlmostEqual(IMP.algebra.get_distance(
            vecs_mean,
            self.scene_centroid), 0., delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
