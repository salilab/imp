import math
import IMP
import IMP.test
import IMP.em
import IMP.algebra


class Tests(IMP.test.TestCase):

    def _expected(self, m, pos):
        """Trilinear interpolation of map m at position pos"""
        h = m.get_header()
        n = (h.get_nx(), h.get_ny(), h.get_nz())
        s = m.get_spacing()
        o = m.get_origin()
        lo, frac = [], []
        for i in range(3):
            ind = (pos[i] - o[i]) / s
            ind = min(max(ind, 0.), n[i] - 1.)
            i0 = min(int(math.floor(ind)), n[i] - 2)
            lo.append(i0)
            frac.append(ind - i0)
        val = 0.
        for dz in (0, 1):
            for dy in (0, 1):
                for dx in (0, 1):
                    w = ((frac[0] if dx else 1 - frac[0])
                         * (frac[1] if dy else 1 - frac[1])
                         * (frac[2] if dz else 1 - frac[2]))
                    val += w * m.get_value(m.xyz_ind2voxel(
                        lo[0] + dx, lo[1] + dy, lo[2] + dz))
        return val

    def test_interpolate_edges(self):
        """interpolate_map should stay within the input map at the edges"""
        # With this origin and spacing, rounding puts the last output voxel
        # just past the end of the input map
        m = IMP.em.create_density_map(10, 10, 10, 1.0)
        m.set_origin(IMP.algebra.Vector3D(2., 2., 2.))
        for i in range(m.get_number_of_voxels()):
            m.set_value(i, float(i % 7) + 0.25 * (i % 13))
        r = IMP.em.interpolate_map(m, 1.1)
        for i in range(r.get_number_of_voxels()):
            v = r.get_value(i)
            self.assertFalse(math.isnan(v))
            self.assertAlmostEqual(
                v, self._expected(m, r.get_location_by_voxel(i)), delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
