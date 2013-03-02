import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to testing coversion from em to particles"""

    def load_density_map(self):
        ### Note: This test used to work with in.em, we moved to mrc
        ### as it was not clear how to save origin in em format.
        ### Resolve with Frido.
        self.mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name("in.mrc"), self.mrw)
        self.scene_threshold=0.01
        self.scene_centroid=self.scene.get_centroid()
    def test_covert_to_particles(self):
        mdl = IMP.Model()
        ps = IMP.em.density2particles(self.scene,self.scene_threshold,mdl)
        self.assertAlmostEqual(
            IMP.algebra.get_distance(IMP.core.get_centroid(ps),
                                     self.scene_centroid),
            0,
            delta=0.1)


    def test_covert_to_vectors(self):
        mdl = IMP.Model()
        vecs = IMP.em.density2vectors(self.scene,self.scene_threshold)
        vecs_mean=IMP.algebra.Vector3D(0.,0.,0.)
        for v in vecs:
            vecs_mean = vecs_mean+v
        vecs_mean=vecs_mean/len(vecs)
        self.assertAlmostEqual(IMP.algebra.get_distance(
            vecs_mean,
            self.scene_centroid),0., delta=0.1)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.load_density_map()
if __name__ == '__main__':
    IMP.test.main()
