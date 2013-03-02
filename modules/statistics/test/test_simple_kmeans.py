import IMP.test
import IMP
import IMP.statistics
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_kmeans(self):
        """Kmeans clustering"""
        vs= []
        centers=(IMP.algebra.Vector3D(0,0,0),
                 IMP.algebra.Vector3D(10,15,20),
                 IMP.algebra.Vector3D(60,30,12))
        for i in range(0,3):
            for j in range(0,100):
                vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 10)))
        e= IMP.statistics.VectorDEmbedding(vs)
        c= IMP.statistics.create_lloyds_kmeans(e,
                                               3, 1000)
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

if __name__ == '__main__':
    IMP.test.main()
