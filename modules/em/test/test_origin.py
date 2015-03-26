import IMP
import IMP.em
import IMP.test
import IMP.algebra

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
        # Voxel 2 should be centered at the point (2,2,3)
        self.assertLess(IMP.algebra.get_distance(IMP.algebra.Vector3D(2,2,3),
                                         d.get_location_by_voxel(2)), 1e-4)
        # All points in a cube of side 0.5 centered at (2,2,3) should be
        # in voxel 2 (indices 2,0,0)
        for i in range(3):
            for offset in (-0.2, 0., 0.2):
                loc = IMP.algebra.Vector3D(2,2,3)
                loc[i] += offset
                self.assertEqual(d.get_voxel_by_location(loc), 2)
                dim_index = [2,0,0]
                for j in range(3):
                    self.assertEqual(d.get_dim_index_by_location(loc[j], j),
                                     dim_index[j])

if __name__ == '__main__':
    IMP.test.main()
