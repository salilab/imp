import IMP.test
import IMP
import IMP.statistics
import IMP.algebra

class Tests(IMP.test.TestCase):
    def _assert_same_bin(self, c, vs, wid):
        for a in c:
            for b in c:
                d= IMP.algebra.get_distance(vs[a], vs[b])
                self.assertLess(d, 2*wid)
    def test_connectivity_clustering(self):
        """Test bin clustering"""
        bb= IMP.algebra.BoundingBoxKD(IMP.algebra.VectorKD(0,0,0,0),
                                      IMP.algebra.VectorKD(10,10,10,10))
        vs=[]
        for i in range(0,1000):
            vs.append(IMP.algebra.get_random_vector_in(bb))
        embed= IMP.statistics.VectorDEmbedding(vs)
        cluster= IMP.statistics.create_bin_based_clustering(embed, 2)
        found=[]
        for i in range(cluster.get_number_of_clusters()):
            c= cluster.get_cluster(i)
            self._assert_same_bin(c, vs, 2)

if __name__ == '__main__':
    IMP.test.main()
