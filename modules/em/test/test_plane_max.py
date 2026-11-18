import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):

    def _make_map(self, nx, ny, nz):
        m = IMP.em.create_density_map(nx, ny, nz, 1.0)
        for i in range(m.get_number_of_voxels()):
            m.set_value(i, float(i))
        return m

    def _max(self, m, axis, ind):
        h = m.get_header()
        n = (h.get_nx(), h.get_ny(), h.get_nz())
        vals = []
        for z in range(n[2]):
            for y in range(n[1]):
                for x in range(n[0]):
                    if (x, y, z)[axis] == ind:
                        vals.append(m.get_value(m.xyz_ind2voxel(x, y, z)))
        return max(vals)

    def test_plane_max(self):
        """Check maximum value in planes of non-cubic maps"""
        for dims in ((8, 2, 3), (2, 8, 3), (3, 2, 8)):
            m = self._make_map(*dims)
            for ind in range(dims[0]):
                self.assertAlmostEqual(m.get_maximum_value_in_yz_plane(ind),
                                       self._max(m, 0, ind), delta=1e-6)
            for ind in range(dims[1]):
                self.assertAlmostEqual(m.get_maximum_value_in_xz_plane(ind),
                                       self._max(m, 1, ind), delta=1e-6)
            for ind in range(dims[2]):
                self.assertAlmostEqual(m.get_maximum_value_in_xy_plane(ind),
                                       self._max(m, 2, ind), delta=1e-6)

    def test_plane_out_of_range(self):
        """Out-of-range plane indices should be rejected"""
        m = self._make_map(2, 8, 3)
        self.assertRaisesUsageException(m.get_maximum_value_in_yz_plane, 2)
        self.assertRaisesUsageException(m.get_maximum_value_in_yz_plane, -1)
        self.assertRaisesUsageException(m.get_maximum_value_in_xz_plane, 8)
        self.assertRaisesUsageException(m.get_maximum_value_in_xz_plane, -1)
        self.assertRaisesUsageException(m.get_maximum_value_in_xy_plane, 3)
        self.assertRaisesUsageException(m.get_maximum_value_in_xy_plane, -1)


if __name__ == '__main__':
    IMP.test.main()
