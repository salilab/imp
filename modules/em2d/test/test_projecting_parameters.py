from __future__ import print_function
import IMP
import IMP.test
import IMP.em2d
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of ProjectingParameters"""
        pp = IMP.em2d.ProjectingParameters(1.5, 40.0)
        dump = pickle.dumps(pp)
        newpp = pickle.loads(dump)
        self.assertAlmostEqual(pp.pixel_size, 1.5, delta=1e-4)
        self.assertAlmostEqual(pp.resolution, 40.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
