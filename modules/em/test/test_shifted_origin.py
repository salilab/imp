import math
import random
import IMP
import IMP.test
import IMP.em
import IMP.algebra


def _make_map(n, origin, values):
    m = IMP.em.create_density_map(n[0], n[1], n[2], 1.0)
    m.set_origin(IMP.algebra.Vector3D(*origin))
    for i, v in enumerate(values):
        m.set_value(i, v)
    return m


def _overlap_pairs(m1, m2):
    """Get (m1 index, m2 index) pairs of voxels at the same position"""
    h1, h2 = m1.get_header(), m2.get_header()
    n1 = (h1.get_nx(), h1.get_ny(), h1.get_nz())
    n2 = (h2.get_nx(), h2.get_ny(), h2.get_nz())
    shift = [int(round(m2.get_origin()[i] - m1.get_origin()[i]))
             for i in range(3)]
    for z in range(n2[2]):
        for y in range(n2[1]):
            for x in range(n2[0]):
                x1, y1, z1 = x + shift[0], y + shift[1], z + shift[2]
                if 0 <= x1 < n1[0] and 0 <= y1 < n1[1] and 0 <= z1 < n1[2]:
                    yield (m1.xyz_ind2voxel(x1, y1, z1),
                           m2.xyz_ind2voxel(x, y, z))


class Tests(IMP.test.TestCase):

    def _make_wrap_maps(self):
        """Make two maps where a linear index shift would wrap around"""
        n = (4, 4, 4)
        # m1 has density only at voxel (0,1,0)
        v1 = [0.] * 64
        v1[4] = 1.0
        # m2 has density only at voxel (3,0,0), and its origin is shifted
        # by one voxel in x, so this voxel lies outside of m1
        v2 = [0.] * 64
        v2[3] = 1.0
        return _make_map(n, (0, 0, 0), v1), _make_map(n, (1, 0, 0), v2)

    def test_convolute_no_wrap(self):
        """convolute() should not wrap across rows"""
        m1, m2 = self._make_wrap_maps()
        self.assertAlmostEqual(IMP.em.convolute(m1, m2), 0.0, delta=1e-8)

    def test_coarse_convolution_no_wrap(self):
        """CoarseConvolution should not wrap across rows"""
        m1, m2 = self._make_wrap_maps()
        self.assertAlmostEqual(
            IMP.em.CoarseConvolution.convolution(m1, m2, -1.0), 0.0,
            delta=1e-8)

    def _make_random_maps(self, n2=(5, 6, 7)):
        random.seed(42)
        n1 = (5, 6, 7)
        m1 = _make_map(n1, (0, 0, 0),
                       [random.random() for _ in range(n1[0]*n1[1]*n1[2])])
        m2 = _make_map(n2, (2, -1, 1),
                       [random.random() for _ in range(n2[0]*n2[1]*n2[2])])
        return m1, m2

    def test_convolute_random(self):
        """Check convolute() of maps with different origins"""
        m1, m2 = self._make_random_maps()
        expected = sum(m1.get_value(i) * m2.get_value(j)
                       for i, j in _overlap_pairs(m1, m2))
        self.assertAlmostEqual(IMP.em.convolute(m1, m2), expected,
                               delta=1e-6)
        self.assertAlmostEqual(
            IMP.em.CoarseConvolution.convolution(m1, m2, -1.0), expected,
            delta=1e-4)

    def test_convolute_different_size(self):
        """Check convolute() of maps with different sizes"""
        m1, m2 = self._make_random_maps(n2=(2, 3, 2))
        expected = sum(m1.get_value(i) * m2.get_value(j)
                       for i, j in _overlap_pairs(m1, m2))
        self.assertAlmostEqual(IMP.em.convolute(m1, m2), expected,
                               delta=1e-6)

    def test_coarse_cc_coefficient(self):
        """Check get_coarse_cc_coefficient of maps with different origins"""
        m1, m2 = self._make_random_maps()
        m1.calcRMS()
        m2.calcRMS()
        thresh = 0.3
        h1, h2 = m1.get_header(), m2.get_header()
        ccc = sum(m1.get_value(i) * m2.get_value(j)
                  for i, j in _overlap_pairs(m1, m2)
                  if m2.get_value(j) > thresh)
        nvox = m1.get_number_of_voxels()
        expected = ((ccc - nvox * h1.dmean * h2.dmean)
                    / (nvox * h1.rms * h2.rms))
        self.assertAlmostEqual(
            IMP.em.get_coarse_cc_coefficient(m1, m2, thresh, False),
            expected, delta=1e-5)

    def test_coarse_cc_local_coefficient(self):
        """Check get_coarse_cc_local_coefficient with different origins"""
        m1, m2 = self._make_random_maps()
        m2.calcRMS()
        thresh = 0.3
        pairs = [(m1.get_value(i), m2.get_value(j))
                 for i, j in _overlap_pairs(m1, m2)
                 if m2.get_value(j) > thresh]
        n = len(pairs)
        mean1 = sum(p[0] for p in pairs) / n
        mean2 = sum(p[1] for p in pairs) / n
        ccc = sum((a - mean1) * (b - mean2) for a, b in pairs)
        rms1 = math.sqrt(sum((a - mean1) ** 2 for a, b in pairs) / n)
        rms2 = math.sqrt(sum((b - mean2) ** 2 for a, b in pairs) / n)
        expected = ccc / (n * rms1 * rms2)
        self.assertAlmostEqual(
            IMP.em.get_coarse_cc_local_coefficient(m1, m2, thresh),
            expected, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
