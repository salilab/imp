import IMP
import IMP.test
import IMP.em2d
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of PolarResamplingParameters"""
        p = IMP.em2d.PolarResamplingParameters(3, 4)
        p.set_estimated_number_of_angles(5)
        p.create_maps_for_resampling()
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)
        self.assertEqual(p.get_number_of_angles(), newp.get_number_of_angles())
        self.assertAlmostEqual(p.get_angle_step(), newp.get_angle_step(),
                               delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
