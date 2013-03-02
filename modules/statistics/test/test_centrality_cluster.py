import IMP.test
import IMP
import IMP.statistics
import IMP.algebra
import random
class Tests(IMP.test.TestCase):
    "Test that the histogram functionality works"
    def Setup(self):
        IMP.test.TestCase.setUp(self)
    def test_histogram_construction(self):
        """Check that the centrality clustering works on trivial sets"""
        s= [IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 10),
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(30,0,0), 10)]
        pts=[]
        for i in range(100):
            pts.append(IMP.algebra.get_random_vector_in(s[i%2]))
        e= IMP.statistics.EuclideanMetric(IMP.statistics.VectorDEmbedding(pts))
        c= IMP.statistics.create_centrality_clustering(e, 20, 2)
        self.assertEqual(c.get_number_of_clusters(), 2)
        for i in range(0, c.get_number_of_clusters()):
            cl= c.get_cluster(i)
            mod = cl[0]%2
            for j in cl:
                cm= j%2
                self.assertEqual(cm, mod)
if __name__ == '__main__':
    IMP.test.main()
