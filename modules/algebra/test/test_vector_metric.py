import IMP
import IMP.test
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_euc_vector_metric(self):
        """Check Euclidean vector metric"""
        m = IMP.algebra.EuclideanVectorKDMetric("foo")
        V = IMP.algebra.VectorKD
        vs = [ V(1,2,3), V(5,6,7), V(10,11,12) ]
        self.assertAlmostEqual(m.get_distance(vs[0], vs[1]), 6.928, delta=1e-3)
        self.assertLess(IMP.algebra.get_distance(m.get_centroid(vs),
                                             V(5.33333,6.33333,7.33333)), 1e-4)

    def test_max_vector_metric(self):
        """Check max vector metric"""
        m = IMP.algebra.MaxVectorKDMetric("foo")
        V = IMP.algebra.VectorKD
        vs = [ V(1,2,3), V(5,6,7), V(10,11,12) ]
        self.assertAlmostEqual(m.get_distance(vs[0], vs[1]), 4.0, delta=1e-3)
        self.assertLess(IMP.algebra.get_distance(m.get_centroid(vs),
                                             V(5.5,6.5,7.5)), 1e-4)

if __name__ == '__main__':
    IMP.test.main()
