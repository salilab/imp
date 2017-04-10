from __future__ import print_function
import unittest
import RMF


class GenericTest(unittest.TestCase):

    def _create(self, path):
        fh = RMF.create_rmf_file(path)
        fh.add_frame("root", RMF.FRAME)
        rt = fh.get_root_node()
        pf = RMF.ParticleFactory(fh)
        reps = [rt.add_child("rep" + str(i), RMF.REPRESENTATION)
                for i in range(0, 5)]
        for i, r in enumerate(reps):
            pd = pf.get(r)
            pd.set_coordinates(RMF.Vector3(0, i, 0))
            pd.set_mass(1)
            pd.set_radius(.5)
        sf = RMF.ScoreFactory(fh)
        rf = RMF.RepresentationFactory(fh)
        fn = rt.add_child("feature", RMF.FEATURE)
        sd = sf.get(fn)
        sd.set_score(10.0)
        rd = rf.get(fn)
        rd.set_representation(reps)

    def _test(self, path):
        fh = RMF.open_rmf_file_read_only(path)
        fh.set_current_frame(RMF.FrameID(0))
        rt = fh.get_root_node()
        ch = rt.get_children()
        fn = ch[-1]
        reps = [x for x in ch[:-1]]
        sf = RMF.ScoreFactory(fh)
        rf = RMF.RepresentationFactory(fh)
        sd = sf.get(fn)
        rd = rf.get(fn)
        print(sd.get_score())
        print(reps)
        print(rd.get_representation())
        self.assert_(sd.get_score() == 10)
        for r0, r1 in zip(rd.get_representation(), reps):
            self.assertEqual(r0, r1)
            #self.assert_(rd.get_representation() == reps)

    def test_multiparent(self):
        """Test that feature nodes work right"""
        RMF.set_log_level("trace")
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("feature." + suffix)
            print("file is", path)
            self._create(path)
            self._test(path)

if __name__ == '__main__':
    unittest.main()
