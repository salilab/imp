import IMP
import IMP.test
import IMP.em2d
import pickle


class Tests(IMP.test.TestCase):
    def test_pickle(self):
        """Test (un-)pickle of ClusterSet"""
        s = IMP.em2d.ClusterSet(10)
        s.do_join_clusters(1, 2, 42.0)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        mat = news.get_linkage_matrix()
        self.assertEqual(len(mat), 1)
        id1, id2, dist = mat[0]
        self.assertEqual(int(id1), 1)
        self.assertEqual(int(id2), 2)
        self.assertAlmostEqual(dist, 42.0, delta=1e-4)

    def test_linkage_pickle(self):
        """Test (un-)pickle of linkage classes"""
        sl = IMP.em2d.SingleLinkage()
        cl = IMP.em2d.CompleteLinkage()
        adl = IMP.em2d.AverageDistanceLinkage()
        dump = pickle.dumps((sl, cl, adl))
        newsl, newcl, newadl = pickle.loads(dump)


if __name__ == '__main__':
    IMP.test.main()
