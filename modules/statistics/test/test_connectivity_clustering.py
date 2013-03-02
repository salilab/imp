import IMP.test
import IMP
import IMP.statistics
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_connectivity_distance(self):
        """Test connectivity clustering distance cutoff"""
        vs = [IMP.algebra.Vector3D(0,0,0.0),
              IMP.algebra.Vector3D(0,0,1.0),
              IMP.algebra.Vector3D(0,0,1.9),
              IMP.algebra.Vector3D(0,0,3.0)]
        e = IMP.statistics.VectorDEmbedding(vs)
        # Distance between points must be *less than* 1.0, not equal to, so
        # we should get exactly three clusters here
        c = IMP.statistics.create_connectivity_clustering(e, 1.0)
        self.assertEqual(c.get_number_of_clusters(), 3)
        # First three points should fall into one cluster
        c = IMP.statistics.create_connectivity_clustering(e, 1.01)
        self.assertEqual(c.get_number_of_clusters(), 2)

    def test_connectivity_clustering(self):
        """Test connectivity clustering"""
        vs= IMP.algebra.Vector3Ds()
        centers=(IMP.algebra.Vector3D(0,0,0),
                 IMP.algebra.Vector3D(10,15,20),
                 IMP.algebra.Vector3D(60,30,12))
        for i in range(0,3):
            for j in range(0,150):
                vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 5)))
        e= IMP.statistics.VectorDEmbedding(vs)
        c= IMP.statistics.create_connectivity_clustering(e, 5)
        self.assertEqual(c.get_number_of_clusters(), 3)
        print c.get_cluster_center(0)
        print c.get_cluster_center(1)
        print c.get_cluster_center(2)

        for i in range(0,3):
            found=False
            for j in range(0,3):
                d=IMP.algebra.get_distance(centers[i],
                                        IMP.algebra.Vector3D(c.get_cluster_center(j)[0],
                                                             c.get_cluster_center(j)[1],
                                                             c.get_cluster_center(j)[2]))
                print d
                if d < 2:
                    found=True
            self.assertTrue(found)
    def test_connectivity_clustering_metric(self):
        """Test metric connectivity clustering"""
        vs= IMP.algebra.Vector3Ds()
        centers=(IMP.algebra.Vector3D(0,0,0),
                 IMP.algebra.Vector3D(10,15,20),
                 IMP.algebra.Vector3D(60,30,12))
        for i in range(0,3):
            for j in range(0,150):
                vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 5)))
        e= IMP.statistics.VectorDEmbedding(vs)
        m= IMP.statistics.EuclideanMetric(e)
        c= IMP.statistics.create_connectivity_clustering(m, 5)
        self.assertEqual(c.get_number_of_clusters(), 3)
        #print c.get_cluster_center(0)
        #print c.get_cluster_center(1)
        #print c.get_cluster_center(2)


if __name__ == '__main__':
    IMP.test.main()
