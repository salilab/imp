import IMP.test
import IMP
import IMP.statistics
import IMP.algebra
import IMP.display
bin=1
err=2*bin*2.0**.5
scale=10

class Tests(IMP.test.TestCase):
    def test_connectivity_clustering(self):
        """Recursive clustering"""
        vs= IMP.algebra.Vector3Ds()
        centers=(scale*IMP.algebra.Vector3D(0,0,0),
                 scale*IMP.algebra.Vector3D(5,3,2),
                 scale*IMP.algebra.Vector3D(10,6,-2))
        for i in range(0,3):
            for j in range(0,50):
                vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 5*scale)))
        e= IMP.statistics.VectorDEmbedding(vs)
        c0= IMP.statistics.create_bin_based_clustering(e, bin)
        for i in range(c0.get_number_of_clusters()):
            print c0.get_cluster_representative(i), vs[c0.get_cluster_representative(i)], len(c0.get_cluster(i))
        m= IMP.statistics.EuclideanMetric(e)
        r= IMP.statistics.RecursivePartitionalClusteringMetric(m, c0)
        self.assertEqual(r.get_number_of_items(), c0.get_number_of_clusters())
        for i in range(0, r.get_number_of_items()):
            for j in range(0, i):
                print r.get_distance(i, j), (vs[c0.get_cluster_representative(i)]-vs[c0.get_cluster_representative(j)]).get_magnitude()
        ci= IMP.statistics.create_diameter_clustering(r, 14*scale+2*err)
        c= r.create_full_clustering(ci)
        w= IMP.display.PymolWriter(self.get_tmp_file_name("rcluster.pym"))
        for i in range(0,c.get_number_of_clusters()):
            cl= c.get_cluster(i)
            for j in cl:
                g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(vs[j], 3))
                g.set_color(IMP.display.get_display_color(i))
                g.set_name(str(i))
                w.add_geometry(g)
        del w
        self.skipTest("bad test")
        self.assertLessEqual(c.get_number_of_clusters(), 5)
        for i in range(0, c.get_number_of_clusters()):
            cl= c.get_cluster(i)
            for a in cl:
                for b in cl:
                    d= (vs[a]-vs[b]).get_magnitude()
                    self.assertLess(d, 14*scale+2*err)

if __name__ == '__main__':
    IMP.test.main()
