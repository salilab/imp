import unittest
import IMP
import IMP.test
import IMP.algebra
import math

class SphereTests(IMP.test.TestCase):

    def test_sphere_construction(self):
        """Check that sphere are constructed correctly"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        radius=5.0
        sph = IMP.algebra.Sphere3D(center,radius)
        self.assertEqual((sph.get_center()-center).get_magnitude() < 0.01,True)
        self.assertEqual(sph.get_radius(),radius)
        self.assertAlmostEqual(sph.get_surface_area(),math.pi*25.0*4,
                               places=1)
        self.assertAlmostEqual(sph.get_volume(),math.pi*125.0*(4.0/3),
                               places=1)

    def test_bounding_cylinder(self):
        """Check correct behavior of Sphere3D.get_bounding_cylinder"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        radius=5.0
        sph = IMP.algebra.Sphere3D(center,radius)
        #print sph.get_bild_string()
        cyl = IMP.algebra.bounding_cylinder(sph)
        #print cyl.get_bild_string()
        self.assertEqual(cyl.get_radius(),radius)
        self.assertEqual(cyl.get_segment().get_length(),2.0*radius)

    def test_sampling_of_bounding_cylinder(self):
        """Check bounding cylinder"""
        center = IMP.algebra.Vector3D(3.0,6.0,2.0)
        radius=5.0
        sph = IMP.algebra.Sphere3D(center,radius)
        cyl=IMP.algebra.bounding_cylinder(sph)
        points = IMP.algebra.grid_cover(cyl,8,8)
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assertEqual(len(points),8*8)
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
        sampled_centroid = sampled_centroid * (1.0/len(points))
        self.assertEqual((sampled_centroid-center).get_magnitude() < 1.0,True)

    def test_enclosing(self):
        """Check enclosing sphere"""
        ss= IMP.algebra.Sphere3Ds()
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        for i in range(0,20):
            v= IMP.algebra.random_vector_in_box(bb)
            r= i
            s= IMP.algebra.Sphere3D(v,r)
            ss.append(s)
        for i in range(1,20):
            css=ss[0:i]
            print len(css)
            es= IMP.algebra.enclosing_sphere(css)
            es.show(); print
            for cs in css:
                d= (cs.get_center()-es.get_center()).get_magnitude()
                cs.show(); print
                self.assert_(d+ cs.get_radius()- es.get_radius() < .5)

if __name__ == '__main__':
    unittest.main()
