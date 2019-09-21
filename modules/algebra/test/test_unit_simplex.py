from __future__ import print_function
import math

import numpy as np

try:
    import scipy.special
except ImportError:
    scipy = None

import IMP
import IMP.test
import IMP.algebra


class UnitSimplexDTests(IMP.test.TestCase):

    types = [
        (1, IMP.algebra.UnitSimplex1D, (), IMP.algebra.Vector1D),
        (2, IMP.algebra.UnitSimplex2D, (), IMP.algebra.Vector2D),
        (3, IMP.algebra.UnitSimplex3D, (), IMP.algebra.Vector3D),
        (4, IMP.algebra.UnitSimplex4D, (), IMP.algebra.Vector4D),
        (5, IMP.algebra.UnitSimplex5D, (), IMP.algebra.Vector5D),
        (6, IMP.algebra.UnitSimplex6D, (), IMP.algebra.Vector6D),
    ]

    types += [
        (d, IMP.algebra.UnitSimplexKD, (d,), IMP.algebra.VectorKD)
        for d in range(1, 11)
    ]

    @staticmethod
    def _get_normal_thresh(tailprob, sigma=1, dim=1):
        alpha = (1 - tailprob) ** dim
        return sigma * np.sqrt(2) * scipy.special.erfinv(alpha)

    def test_construction(self):
        """Check that fixed-dimension simplices are constructed correctly"""
        for d, st, args, vt in self.types:
            s = st(*args)
            self.assertEqual(s.get_dimension(), d)

    def test_construct_kd_with_wrong_dimension_raises_error(self):
        self.assertRaisesUsageException(IMP.algebra.UnitSimplexKD, 0)
        self.assertRaisesUsageException(IMP.algebra.UnitSimplexKD, -1)

    def test_get_barycenter(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            v = s.get_barycenter()
            self.assertIsInstance(v, vt)
            self.assertSequenceAlmostEqual(list(s.get_barycenter()), [1.0 / d] * d)

    def test_get_contains(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            for i in range(10):
                if isinstance(vt, IMP.algebra.VectorKD):
                    v = -np.log(np.random.uniform(size=d + 1))
                    v /= np.sum(v)
                    self.assertFalse(s.get_contains(vt(v)))

                v = -np.log(np.random.uniform(size=d))
                self.assertFalse(s.get_contains(vt(v)))

                v /= np.sum(v)
                self.assertTrue(s.get_contains(vt(v)))

    def test_get_vertices(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            vs = IMP.algebra.get_vertices(s)
            I = np.eye(d)
            self.assertEqual(len(vs), d)
            for i, v in enumerate(vs):
                self.assertIsInstance(v, vt)
                self.assertSequenceAlmostEqual(list(v), list(I[i, :]))

    def test_get_increasing_from_embedded(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            for i in range(10):
                v = -np.log(np.random.uniform(size=d))
                v /= np.sum(v)

                inc = IMP.algebra.get_increasing_from_embedded(s, vt(v))
                self.assertIsInstance(inc, vt)
                self.assertSequenceAlmostEqual(list(inc), list(np.cumsum(v)))

    def test_get_embedded_from_increasing(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            for i in range(10):
                v = -np.log(np.random.uniform(size=d))
                v /= np.sum(v)

                inc = np.cumsum(v)
                v2 = IMP.algebra.get_embedded_from_increasing(s, vt(inc))
                self.assertIsInstance(v2, vt)
                self.assertSequenceAlmostEqual(list(v2), list(v))

    def test_get_projected(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            v = np.random.normal(size=d)
            v_proj = IMP.algebra.get_projected(s, vt(v))
            self.assertIsInstance(v_proj, vt)
            v_proj = np.array(v_proj, dtype=np.double)

            pos_inds = v_proj != 0.0
            vshift = v[pos_inds] - v_proj[pos_inds]

            self.assertTrue(np.all(v_proj >= 0))
            self.assertAlmostEqual(np.sum(v_proj), 1)

            # projection has cut point
            if len(v[~pos_inds]) > 0:
                min_pos = np.amin(v[pos_inds])
                max_zero = np.amax(v[~pos_inds])
                self.assertGreater(min_pos, max_zero)

            # projection is rigid shift
            self.assertSequenceAlmostEqual(
                list(vshift), [vshift[0]] * len(vshift)
            )

    def test_get_random_vector_on(self):
        for d, st, args, vt in self.types:
            s = st(*args)
            for i in range(10):
                v = IMP.algebra.get_random_vector_on(s)
                self.assertIsInstance(v, vt)
                self.assertEqual(v.get_dimension(), d)
                self.assertAlmostEqual(np.sum(v), 1)

    @IMP.test.skipIf(scipy is None, "Requires SciPy")
    def test_get_random_vector_on_is_uniform(self):
        """Test that result of get_random_vector_on is uniform on simplex.

        Checks that each component of the Monte Carlo estimate of the mean
        follows the central limit theorem.
        """
        n = 1000
        fail_prob = 1e-3  # Probability of all tests failing.
        each_fail_prob = 1 - (1 - fail_prob) ** (1.0 / len(self.types))

        for d, st, args, vt in self.types:
            s = st(*args)
            bary_vs = []
            c = s.get_barycenter()
            for i in range(n):
                v = IMP.algebra.get_random_vector_on(s)
                bary_vs.append(np.array(v - c, dtype=np.double))

            if scipy:
                mean_bary_vs = np.mean(bary_vs, axis=0)
                mcse = ((d - 1.0) / (d + 1.0) / n) ** 0.5 / d
                mean_thresh = self._get_normal_thresh(
                    each_fail_prob, dim=d, sigma=mcse
                )
                self.assertTrue(np.all(mean_bary_vs <= mean_thresh))


if __name__ == "__main__":
    IMP.test.main()
