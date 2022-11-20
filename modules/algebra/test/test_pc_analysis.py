import IMP
import IMP.test
import IMP.algebra
import pickle


class Tests(IMP.test.TestCase):

    def test_access(self):
        """Check PrincipalComponentAnalysis3D accessors"""
        exp_comp = [IMP.algebra.Vector3D(1, 1, 1)]
        exp_vals = IMP.algebra.Vector3D(2, 3, 4)
        exp_cent = IMP.algebra.Vector3D(5, 6, 7)
        pca = IMP.algebra.PrincipalComponentAnalysis3D(exp_comp, exp_vals,
                                                       exp_cent)
        comp = pca.get_principal_components()
        self.assertEqual(len(comp), 1)
        self.assertAlmostEqual(IMP.algebra.get_squared_distance(comp[0],
                                                                exp_comp[0]),
                               0.0, delta=1e-5)
        comp = pca.get_principal_component(0)
        self.assertAlmostEqual(IMP.algebra.get_squared_distance(comp,
                                                                exp_comp[0]),
                               0.0, delta=1e-5)
        val = pca.get_principal_values()
        self.assertAlmostEqual(IMP.algebra.get_squared_distance(val, exp_vals),
                               0.0, delta=1e-5)
        self.assertAlmostEqual(pca.get_principal_value(1), 3., delta=1e-5)
        cent = pca.get_centroid()
        self.assertAlmostEqual(
            IMP.algebra.get_squared_distance(cent, exp_cent),
            0.0, delta=1e-5)
        new_cent = IMP.algebra.Vector3D(8, 9, 10)
        pca.set_centroid(new_cent)
        cent = pca.get_centroid()
        self.assertAlmostEqual(
            IMP.algebra.get_squared_distance(cent, new_cent),
            0.0, delta=1e-5)

    def test_compare(self):
        """Check PrincipalComponentAnalysis3D comparison"""
        exp_comp = [IMP.algebra.Vector3D(1, 1, 1)]
        exp_vals = IMP.algebra.Vector3D(2, 3, 4)
        exp_cent = IMP.algebra.Vector3D(5, 6, 7)
        pca = IMP.algebra.PrincipalComponentAnalysis3D(exp_comp, exp_vals,
                                                       exp_cent)
        def_pca = IMP.algebra.PrincipalComponentAnalysis3D()
        # Note: currently at least one of the objects being compared
        # must be default constructed
        self.assertEqual(def_pca, def_pca)
        self.assertNotEqual(pca, def_pca)

    def test_get_pca(self):
        """Check get_principal_components()"""
        pca = IMP.algebra.get_principal_components(
            [IMP.algebra.Vector3D(0, 0, 1), IMP.algebra.Vector3D(1, 0, 0)])
        exp_vecs = [IMP.algebra.Vector3D(0.707107, 0, -0.707107),
                    IMP.algebra.Vector3D(0, 1, 0),
                    IMP.algebra.Vector3D(0.707107, 0, 0.707107)]
        vecs = pca.get_principal_components()
        for i, ev in enumerate(exp_vecs):
            self.assertAlmostEqual(IMP.algebra.get_squared_distance(ev,
                                                                    vecs[i]),
                                   0.0, delta=1e-5)
        vals = pca.get_principal_values()
        self.assertAlmostEqual(IMP.algebra.get_squared_distance(
            vals, IMP.algebra.Vector3D(1, 0, 0)),
            0.0, delta=1e-5)
        cent = pca.get_centroid()
        self.assertAlmostEqual(IMP.algebra.get_squared_distance(
            cent, IMP.algebra.Vector3D(
                0.5, 0, 0.5)),
            0.0, delta=1e-5)

    def test_pickle(self):
        """Test (un-)pickled of PrincipalComponentAnalysis3D"""
        exp_comp = [IMP.algebra.Vector3D(1, 1, 1)]
        exp_vals = IMP.algebra.Vector3D(2, 3, 4)
        exp_cent = IMP.algebra.Vector3D(5, 6, 7)
        p1 = IMP.algebra.PrincipalComponentAnalysis3D(exp_comp, exp_vals,
                                                      exp_cent)
        exp_comp = [IMP.algebra.Vector3D(5, 6, 7)]
        exp_vals = IMP.algebra.Vector3D(8, 9, 10)
        exp_cent = IMP.algebra.Vector3D(12, 13, 14)
        p2 = IMP.algebra.PrincipalComponentAnalysis3D(exp_comp, exp_vals,
                                                      exp_cent)
        p2.foo = 'bar'
        dump = pickle.dumps((p1, p2))
        newp1, newp2 = pickle.loads(dump)
        self._check_equal(p1, newp1)
        self._check_equal(p2, newp2)
        self.assertEqual(newp2.foo, 'bar')

        self.assertRaises(TypeError, p1._set_from_binary, 42)

    def _check_equal(self, a, b):
        compa = a.get_principal_components()
        compb = b.get_principal_components()
        self.assertEqual(len(compa), len(compb))
        for ca, cb in zip(compa, compb):
            self.assertLess(IMP.algebra.get_distance(ca, cb), 1e-5)
        self.assertLess(IMP.algebra.get_distance(
            a.get_principal_values(), b.get_principal_values()), 1e-5)
        self.assertLess(IMP.algebra.get_distance(
            a.get_centroid(), b.get_centroid()), 1e-5)


if __name__ == '__main__':
    IMP.test.main()
