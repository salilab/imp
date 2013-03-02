import IMP.test
import IMP
import IMP.statistics
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_connectivity_clustering(self):
        """Diameter clustering"""
        vs= IMP.algebra.Vector3Ds()
        centers=(IMP.algebra.Vector3D(0,0,0),
                 IMP.algebra.Vector3D(5,3,2),
                 IMP.algebra.Vector3D(10,6,-2))
        for i in range(0,3):
            for j in range(0,50):
                vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 5)))
        e= IMP.statistics.VectorDEmbedding(vs)
        m= IMP.statistics.EuclideanMetric(e)
        c= IMP.statistics.create_diameter_clustering(m, 14)
        #self.assert_(c.get_number_of_clusters()<= 5)

        for i in range(0, c.get_number_of_clusters()):
            cl= c.get_cluster(i)
            for a in cl:
                for b in cl:
                    d= (vs[a]-vs[b]).get_magnitude()
                    self.assertLess(d, 14)

if __name__ == '__main__':
    IMP.test.main()
