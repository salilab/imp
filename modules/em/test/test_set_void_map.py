import IMP
import IMP.test
import IMP.em
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_set_void_map_locations(self):
        """set_void_map should update voxel locations"""
        m = IMP.em.create_density_map(2, 2, 2, 2.0)
        m.set_origin(IMP.algebra.Vector3D(1., 2., 3.))
        # Make sure locations are cached for the old size
        self.assertLess(IMP.algebra.get_distance(
            m.get_location_by_voxel(7), IMP.algebra.Vector3D(3., 4., 5.)),
            1e-5)
        m.calcRMS()
        m.set_void_map(5, 6, 7)
        self.assertEqual(m.get_number_of_voxels(), 210)
        self.assertFalse(m.get_rms_calculated())
        for x, y, z in ((4, 5, 6), (1, 2, 3), (0, 0, 0)):
            v = m.xyz_ind2voxel(x, y, z)
            expected = IMP.algebra.Vector3D(1. + 2. * x, 2. + 2. * y,
                                            3. + 2. * z)
            self.assertLess(IMP.algebra.get_distance(
                m.get_location_by_voxel(v), expected), 1e-5)
            self.assertEqual(m.get_value(v), 0.0)


if __name__ == '__main__':
    IMP.test.main()
