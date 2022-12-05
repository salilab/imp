import IMP
import IMP.test
import IMP.em2d
import pickle

class Tests(IMP.test.TestCase):

    def get_pickle(self):
        """Test (un-)pickle of SegmentationParameters"""
        p = IMP.em2d.SegmentationParameters()
        p.image_pixel_size = 2.7
        p.diffusion_timesteps = 0
        p.diffusion_beta = 0
        p.fill_holes_stddevs = 0.5
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)
        self.assertAlmostEqual(newp.image_pixel_size, 2.7, delta=1e-4)
        self.assertAlmostEqual(newp.diffusion_timesteps, 0., delta=1e-4)
        self.assertAlmostEqual(newp.diffusion_beta, 0., delta=1e-4)
        self.assertAlmostEqual(newp.fill_holes_stddevs, 0.5, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
