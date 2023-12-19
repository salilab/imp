import IMP
import IMP.test
import IMP.algebra
import pickle


class Tests(IMP.test.TestCase):

    def test_euc_vector_metric(self):
        """Check Euclidean vector metric"""
        m = IMP.algebra.EuclideanVectorKDMetric("foo")
        V = IMP.algebra.VectorKD
        vs = [V(1, 2, 3), V(5, 6, 7), V(10, 11, 12)]
        self.assertAlmostEqual(m.get_distance(vs[0], vs[1]), 6.928, delta=1e-3)
        self.assertLess(IMP.algebra.get_distance(
            m.get_centroid(vs), V(5.33333, 6.33333, 7.33333)), 1e-4)

    def test_euc_vector_pickle(self):
        """Check (un-)pickle of Euclidean vector metric"""
        m = IMP.algebra.EuclideanVectorKDMetric("foo")
        dump = pickle.dumps(m)
        newm = pickle.loads(dump)
        self.assertEqual(newm.get_name(), "foo")
        V = IMP.algebra.VectorKD
        vs = [V(1, 2, 3), V(5, 6, 7), V(10, 11, 12)]
        self.assertAlmostEqual(newm.get_distance(vs[0], vs[1]),
                               6.928, delta=1e-3)

    def test_euc_vector_pickle_polymorphic(self):
        """Check (un-)pickle of Euclidean vector metric via polymorphic ptr"""
        m = IMP.Model()
        mk = IMP.ModelKey("mk")
        metric = IMP.algebra.EuclideanVectorKDMetric("foo")
        m.add_data(mk, metric)
        dump = pickle.dumps(m)
        newm = pickle.loads(dump)

        newmetric = IMP.algebra.EuclideanVectorKDMetric.get_from(
            newm.get_data(mk))
        self.assertEqual(newmetric.get_name(), "foo")

    def test_max_vector_metric(self):
        """Check max vector metric"""
        m = IMP.algebra.MaxVectorKDMetric("foo")
        V = IMP.algebra.VectorKD
        vs = [V(1, 2, 3), V(5, 6, 7), V(10, 11, 12)]
        self.assertAlmostEqual(m.get_distance(vs[0], vs[1]), 4.0, delta=1e-3)
        self.assertLess(IMP.algebra.get_distance(m.get_centroid(vs),
                                                 V(5.5, 6.5, 7.5)), 1e-4)

    def test_max_vector_metric_pickle(self):
        """Check (un-)pickle of max vector metric"""
        m = IMP.algebra.MaxVectorKDMetric("foo")
        dump = pickle.dumps(m)
        newm = pickle.loads(dump)
        self.assertEqual(newm.get_name(), "foo")
        V = IMP.algebra.VectorKD
        vs = [V(1, 2, 3), V(5, 6, 7), V(10, 11, 12)]
        self.assertAlmostEqual(newm.get_distance(vs[0], vs[1]),
                               4.0, delta=1e-3)

    def test_max_vector_pickle_polymorphic(self):
        """Check (un-)pickle of max vector metric via polymorphic ptr"""
        m = IMP.Model()
        mk = IMP.ModelKey("mk")
        metric = IMP.algebra.MaxVectorKDMetric("foo")
        m.add_data(mk, metric)
        dump = pickle.dumps(m)
        newm = pickle.loads(dump)

        newmetric = IMP.algebra.MaxVectorKDMetric.get_from(
            newm.get_data(mk))
        self.assertEqual(newmetric.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
