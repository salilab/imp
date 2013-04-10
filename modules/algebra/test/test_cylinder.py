import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

    def test_cylinder_construction(self):
        """Check Cylinder3D construction from vectors"""
        print "cylinder"
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        direction = IMP.algebra.Vector3D(0.0,0.0,1.0)
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0,0.0,-4.0),
                                                           IMP.algebra.Vector3D(0.0,0.0,4.0)),
                                   5.0)
        self.assertEqual((cyl.get_segment().get_middle_point()-center).get_magnitude() < 0.01,True)
        self.assertEqual((cyl.get_segment().get_direction()-direction).get_magnitude() < 0.01,True)
        self.assertEqual(cyl.get_radius(),5.0)
        self.assertEqual(cyl.get_segment().get_length(),8.0)

        self.assertAlmostEqual(IMP.algebra.get_surface_area(cyl),2*math.pi*5.0*8.0+2*math.pi*25.0,
                               places=1)
        self.assertAlmostEqual(IMP.algebra.get_volume(cyl),math.pi*8.0*25.0,
                               places=1)

    def test_get_grid_surface_cover(self):
        "Check grid cover with cylinder at origin in Z direction"
        print "zero"
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        direction = IMP.algebra.Vector3D(0.0,0.0,1.0)
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0,0.0,-4.0),
                                                           IMP.algebra.Vector3D(0.0,0.0,4.0)),
                                   5.0)
        points = IMP.algebra.get_grid_surface_cover(cyl,8,8)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),8*8)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)

    def test_get_grid_surface_cover_center_not_at_000(self):
        """Check grid cover when the center of cylinder is not at (0,0,0)"""
        print "center"
        center = IMP.algebra.Vector3D(5.0,4.0,3.0)
        direction = IMP.algebra.Vector3D(0.0,0.0,1.0)
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(5.0,4.0,-2.0),
                                                           IMP.algebra.Vector3D(5.0,4.0,8.0)),
                                   5.0)
        f= self.get_tmp_file_name("cylinder_shifted.pym")
        points = IMP.algebra.get_grid_surface_cover(cyl,8,8)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),8*8)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertLess((sampled_centroid-center).get_magnitude(), .1)

    def test_get_grid_surface_cover_with_direction_not_on_Z(self):
        "Check grid cover when the direction of the cylinder is not the Z axis"
        print "dir"
        center = IMP.algebra.Vector3D(9.0,5.5,3.5)
        direction = IMP.algebra.Vector3D(12.0,3.0,13.0).get_unit_vector()
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(3.0,4.0,-3.0),
                                                           IMP.algebra.Vector3D(15.0,7.0,10.0)),
                                   5.0)
        points=IMP.algebra.get_grid_surface_cover(cyl,12,12)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),12*12)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertLess((sampled_centroid-center).get_magnitude(), .1)


    def test_get_uniform_surface_cover_with_direction_not_on_Z(self):
        """Check uniform cover with cylinder not on the Z axis"""
        print "uniform"
        center = IMP.algebra.Vector3D(9.0,5.5,3.5)
        direction = IMP.algebra.Vector3D(12.0,3.0,13.0).get_unit_vector()
        cyl = IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(3.0,4.0,-3.0),
                                                           IMP.algebra.Vector3D(15.0,7.0,10.0)),
                                   5.0)
        points=IMP.algebra.get_uniform_surface_cover(cyl,1000)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertGreaterEqual(len(points), 1000)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)


    def test_random_cylinder_sampling(self):
        """Check whether the cylinder volume is uniformly sampled"""
        print "sampling"
        # create an origin centered cylinder with length and radius 10.0
        axis =  IMP.algebra.Segment3D( [0,0,-5.0] , [0,0,5.0] )
        radius = 10.0
        c = IMP.algebra.Cylinder3D( axis, radius )
        # count samples that fall within an origin centered 6x6x6 box
        # that is full contained within the cylinder (since 3*3 < 10)
        inner_box = IMP.algebra.BoundingBox3D([-3,-3,-3],[3,3,3])
        n = 0
        m = 100000
        for i in range( m ):
            v = IMP.algebra.get_random_vector_in( c )
            if( inner_box.get_contains( v ) ):
                n = n + 1
        # assert probabilistically (using Chernoff bound, m is good enough)
        from IMP.algebra import get_volume
        expected_p = get_volume(inner_box) / get_volume(c)
        observed_p = float(n) / float(m)
        self.assertAlmostEqual(observed_p, expected_p, delta = .01)


if __name__ == '__main__':
    IMP.test.main()
