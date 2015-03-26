import IMP
import IMP.em
import IMP.test
import IMP.algebra

def get_points_around(v, offset):
    """Yield a set of points offset by `+/- offset` along each axis from v"""
    for i in range(3):
        for off in (-offset, 0., offset):
            loc = IMP.algebra.Vector3D(v)
            loc[i] += off
            yield loc

class Tests(IMP.test.TestCase):

    def test_bounding_box(self):
        """Map should round up range from bounding box"""
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1,2,3),
                                       IMP.algebra.Vector3D(2.9,4.9,7))
        d = IMP.em.create_density_map(bb, 0.5)
        bb = IMP.em.get_bounding_box(d, -1.)
        self.assertEqual(d.get_number_of_voxels(), 192)
        self.assertLess(IMP.algebra.get_distance(IMP.algebra.Vector3D(1,2,3),
                                                 bb.get_corner(0)), 1e-4)
        self.assertLess(IMP.algebra.get_distance(IMP.algebra.Vector3D(3,5,7),
                                                 bb.get_corner(1)), 1e-4)
        # "Left" edge of origin voxel must be at (1,2,3) to cover the original
        # bounding box, so the origin must be offset by half the spacing
        self.assertLess(IMP.algebra.get_distance(
                            IMP.algebra.Vector3D(1.25,2.25,3.25),
                            d.get_origin()), 1e-4)
        # Top (which isn't in the map) must be outside the bounding box
        self.assertLess(IMP.algebra.get_distance(
                              IMP.algebra.Vector3D(3.25,5.25,7.25),
                              d.get_top()), 1e-4)

    def test_num_voxels(self):
        """Check numbering of voxels"""
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0.75,1.75,2.75),
                                       IMP.algebra.Vector3D(2.75,4.75,6.75))
        d = IMP.em.create_density_map(bb, 0.5)
        self.assertEqual(d.get_header().get_nx(), 4)
        self.assertEqual(d.get_header().get_ny(), 6)
        self.assertEqual(d.get_header().get_nz(), 8)
        self.assertEqual(d.get_header().get_number_of_voxels(), 192)
        self.assertEqual(d.xyz_ind2voxel(0,0,0), 0)
        self.assertEqual(d.xyz_ind2voxel(1,1,2), 53)
        for inmap in [(0,0,0), (3,5,7)]:
            self.assertTrue(d.is_xyz_ind_part_of_volume(*inmap))
        for outmap in [(-1,0,0), (4,6,8)]:
            self.assertFalse(d.is_xyz_ind_part_of_volume(*outmap))

    def test_is_part_of_volume(self):
        """Check is_part_of_volume"""
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1,2,3),
                                       IMP.algebra.Vector3D(3,4,7))
        d = IMP.em.create_density_map(bb, 0.5)
        for inmap in [(1,2,3), (2.99,3.99,6.99)]:
            self.assertTrue(d.is_part_of_volume(*inmap))
            self.assertTrue(d.is_part_of_volume(IMP.algebra.Vector3D(*inmap)))
        for outmap in [(3,4,7), (0.99,2,3)]:
            self.assertFalse(d.is_part_of_volume(*outmap))
            self.assertFalse(d.is_part_of_volume(IMP.algebra.Vector3D(*outmap)))

    def test_get_set_value(self):
        """Check get_value and set_value"""
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0.75,1.75,2.75),
                                       IMP.algebra.Vector3D(2.75,4.75,6.75))
        d = IMP.em.create_density_map(bb, 0.5)
        # Voxel 2 should be centered at the point (2,2,3)
        d.set_value(2, 16.)
        self.assertAlmostEqual(d.get_value(2), 16., delta=1e-4)
        for loc in get_points_around(IMP.algebra.Vector3D(2,2,3), 0.2):
            self.assertAlmostEqual(d.get_value(loc), 16., delta=1e-4)
            self.assertAlmostEqual(d.get_value(loc[0], loc[1], loc[2]),
                                   16., delta=1e-4)
        d.set_value(2., 2., 3., 32.)
        self.assertAlmostEqual(d.get_value(2), 32., delta=1e-4)

    def test_locations(self):
        """Check mapping from Cartesian coordinates to and from voxel indices"""
        origin = IMP.algebra.Vector3D(1,2,3)
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0.75,1.75,2.75),
                                       IMP.algebra.Vector3D(2.75,4.75,6.75))
        d = IMP.em.create_density_map(bb, 0.5)
        # Check origin just to be sure
        self.assertLess(IMP.algebra.get_distance(origin, d.get_origin()), 1e-4)
        # Voxel 0 should be centered at the origin (1,2,3)
        for i in range(3):
            self.assertAlmostEqual(d.get_location_in_dim_by_voxel(0, i),
                                   origin[i], delta=1e-4)
        self.assertLess(IMP.algebra.get_distance(origin,
                                         d.get_location_by_voxel(0)), 1e-4)
        if IMP.base.get_check_level() >= IMP.base.USAGE:
            self.assertRaises(IMP.base.UsageException,
                              d.get_location_by_voxel, -1)
            self.assertRaises(IMP.base.UsageException,
                              d.get_location_by_voxel, 192)
        # Voxel 2 should be centered at the point (2,2,3)
        self.assertLess(IMP.algebra.get_distance(IMP.algebra.Vector3D(2,2,3),
                                         d.get_location_by_voxel(2)), 1e-4)
        # All points in a cube of side 0.5 centered at (2,2,3) should be
        # in voxel 2 (indices 2,0,0)
        for loc in get_points_around(IMP.algebra.Vector3D(2,2,3), 0.2):
            self.assertEqual(d.get_voxel_by_location(loc), 2)
            dim_index = [2,0,0]
            for j in range(3):
                self.assertEqual(d.get_dim_index_by_location(loc[j], j),
                                 dim_index[j])

if __name__ == '__main__':
    IMP.test.main()
