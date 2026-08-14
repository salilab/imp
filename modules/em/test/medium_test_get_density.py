import IMP.test
import IMP.em
import random


class Tests(IMP.test.TestCase):

    def get_center(self, m, i, j, k):
        return IMP.algebra.Vector3D(m.get_origin()[0] + m.get_spacing() * i,
                                    m.get_origin()[1] + m.get_spacing() * j,
                                    m.get_origin()[2] + m.get_spacing() * k)

    def test_image(self):
        """Check get_density"""
        m = IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        wid = m.get_top() - m.get_origin()
        for i in random.sample(range(0, m.get_header().get_nx()), 30):
            # print i
            for j in random.sample(range(0, m.get_header().get_ny()), 30):
                for k in random.sample(range(0, m.get_header().get_nz()), 30):
                    v = self.get_center(m, i, j, k)
                    # print v
                    val = IMP.em.get_density(m, v)
                    mval = m.get_value(m.xyz_ind2voxel(i, j, k))
                    self.assertAlmostEqual(
                        val, mval, delta=(val + mval) * .1 + .1)
                    self.assertEqual(
                        IMP.em.get_density(
                            m, self.get_center(m, i, j, k)
                            + IMP.algebra.Vector3D(wid[0], 0, 0)), 0)
                    self.assertEqual(
                        IMP.em.get_density(
                            m, self.get_center(m, i, j, k)
                            + IMP.algebra.Vector3D(0, wid[1], 0)), 0)
                    self.assertEqual(
                        IMP.em.get_density(
                            m, self.get_center(m, i, j, k)
                            + IMP.algebra.Vector3D(0, 0, wid[2])), 0)
                    self.assertEqual(
                        IMP.em.get_density(
                            m, self.get_center(m, i, j, k)
                            + IMP.algebra.Vector3D(-wid[0], 0, 0)), 0)
                    self.assertEqual(
                        IMP.em.get_density(
                            m, self.get_center(m, i, j, k)
                            + IMP.algebra.Vector3D(0, -wid[1], 0)), 0)
                    self.assertEqual(
                        IMP.em.get_density(
                            m, self.get_center(m, i, j, k)
                            + IMP.algebra.Vector3D(0, 0, -wid[2])), 0)

        # Choose a random voxel in the grid. Avoid the far edge so that
        # we don't go outside the grid in our simple interpolation below
        indexes = [random.randint(0, m.get_header().get_nx() - 2),
                   random.randint(0, m.get_header().get_ny() - 2),
                   random.randint(0, m.get_header().get_nz() - 2)]
        # Choose a random point in the voxel
        fs = IMP.algebra.Vector3D(random.uniform(0, 1),
                                  random.uniform(0, 1),
                                  random.uniform(0, 1))
        side = m.get_spacing()
        prod = fs * side
        pt = self.get_center(m, indexes[0], indexes[1], indexes[2]) + prod
        val = IMP.em.get_density(m, pt)
        # get_density interpolates based on surrounding voxels. Check this
        # by verifying that the density value is within the range spanned
        # by these voxels.
        lb = 100000000
        ub = -lb
        for i in range(0, 2):
            for j in range(0, 2):
                for k in range(0, 2):
                    p = m.get_value(
                        m.xyz_ind2voxel(indexes[0] + i, indexes[1] + j,
                                        indexes[2] + k))
                    if p < lb:
                        lb = p
                    if p > ub:
                        ub = p

        # Tolerate the density exceeding the bounds by 1e-6 to allow for
        # limited machine precision
        self.assertTrue(val <= ub + 1e-6 and val >= lb - 1e-6,
                        "Density %g is not within bounds %g to %g"
                        % (val, lb, ub))


if __name__ == '__main__':
    IMP.test.main()
