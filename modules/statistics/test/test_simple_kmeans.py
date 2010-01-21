import unittest
import IMP.test
import IMP
import IMP.statistics
import IMP.core
import IMP.algebra

class KMeansTests(IMP.test.TestCase):
    def test_kmeans(self):
        vs= IMP.algebra.Vector3Ds()
        centers=(IMP.algebra.Vector3D(0,0,0),
                 IMP.algebra.Vector3D(10,15,20),
                 IMP.algebra.Vector3D(60,30,12))
        for i in range(0,3):
            for j in range(0,50):
                vs.push_back(IMP.algebra.random_vector_in_sphere(centers[i], 10))
        e= IMP.statistics.Vector3DEmbedding(vs)
        c= IMP.statistics.get_lloyds_kmeans(e,
                                            3, 1000)
        print c.get_cluster_center(0)
        print c.get_cluster_center(1)
        print c.get_cluster_center(2)

        for i in range(0,3):
            found=False
            for j in range(0,3):
                d=IMP.algebra.distance(centers[i],
                                        IMP.algebra.Vector3D(c.get_cluster_center(j)[0],
                                                             c.get_cluster_center(j)[1],
                                                             c.get_cluster_center(j)[2]))
                print d
                if d < 2:
                    found=True
            self.assert_(found)

if __name__ == '__main__':
    unittest.main()
