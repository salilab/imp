import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra
import math

class CylinderTests(IMP.test.TestCase):

    def test_cylinder_construction(self):
        """Check Cylinder3D construction from vectors"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        direction = IMP.algebra.Vector3D(0.0,0.0,1.0)
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0,0.0,-4.0),
                                                           IMP.algebra.Vector3D(0.0,0.0,4.0)),
                                   5.0);
        self.assertEqual((cyl.get_segment().get_middle_point()-center).get_magnitude() < 0.01,True)
        self.assertEqual((cyl.get_segment().get_direction()-direction).get_magnitude() < 0.01,True)
        self.assertEqual(cyl.get_radius(),5.0)
        self.assertEqual(cyl.get_segment().get_length(),8.0)

        self.assertAlmostEqual(cyl.get_surface_area(),2*math.pi*5.0*8.0+2*math.pi*25.0,
                               places=1)
        self.assertAlmostEqual(cyl.get_volume(),math.pi*8.0*25.0,
                               places=1)

    def test_grid_cover(self):
        """Check grid cover when the direction of the
           cylinder is on Z and the center is at (0,0,0)"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        direction = IMP.algebra.Vector3D(0.0,0.0,1.0)
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0,0.0,-4.0),
                                                           IMP.algebra.Vector3D(0.0,0.0,4.0)),
                                   5.0);
        points = IMP.algebra.grid_cover(cyl,8,8)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),8*8)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)

    def test_grid_cover_center_not_at_000(self):
        """Check grid cover when the center of cylinder
           is not at (0,0,0)"""
        center = IMP.algebra.Vector3D(5.0,4.0,2.0)
        direction = IMP.algebra.Vector3D(0.0,0.0,1.0)
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(5.0,4.0,-2.0),
                                                           IMP.algebra.Vector3D(5.0,4.0,8.0)),
                                   5.0);
        points = IMP.algebra.grid_cover(cyl,8,8)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),8*8)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)

    def test_grid_cover_with_direction_not_on_Z(self):
        """Check grid cover when the direction of the
           cylinde is not the Z axis"""
        center = IMP.algebra.Vector3D(9.0,5.5,3.5)
        direction = IMP.algebra.Vector3D(12.0,3.0,13.0).get_unit_vector()
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(3.0,4.0,-3.0),
                                                           IMP.algebra.Vector3D(15.0,7.0,10.0)),
                                   5.0);

        points=IMP.algebra.grid_cover(cyl,12,12)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),12*12)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)


    def test_uniform_cover_with_direction_not_on_Z(self):
        """Check uniform cover with cylinder not on the Z axis"""
        center = IMP.algebra.Vector3D(9.0,5.5,3.5)
        direction = IMP.algebra.Vector3D(12.0,3.0,13.0).get_unit_vector()
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(3.0,4.0,-3.0),
                                                           IMP.algebra.Vector3D(15.0,7.0,10.0)),
                                   5.0);
        points=IMP.algebra.uniform_cover(cyl,1000)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),1000)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)


if __name__ == '__main__':
    unittest.main()
