import unittest
import IMP
import IMP.test
import IMP.algebra
import math

class SphereTests(IMP.test.TestCase):
    def test_uniform_cover(self):
        """Check uniform cover on a sphere"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        radius=6.0
        sph = IMP.algebra.Sphere3D(center,radius)
        numpts=20
        points=IMP.algebra.uniform_cover(sph,numpts)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assert_(len(points)>=numpts)

        print "the sample 1"
        for p in points:
            sampled_centroid = sampled_centroid + p
            print ".sphere " + str(p[0])+ " " + str(p[1]) + " " + str(p[2])\
                + " .1"
        sampled_centroid.show()
        sampled_centroid = sampled_centroid * (1.0/len(points))
        sampled_centroid.show()
        self.assertInTolerance((sampled_centroid-center).get_magnitude(),0,
                               4*radius/numpts**.5)


    def test_uniform_cover_not_on_000(self):
        """Check uniform cover when the the center is not on (0,0,0)"""
        #IMP.random_number_generator.seed(1)
        center = IMP.algebra.Vector3D(4.0,5.0,-9.0)
        radius=5.0
        sph = IMP.algebra.Sphere3D(center,radius)
        nump=40
        if not IMP.has_cgal:
            nump=400
        points=IMP.algebra.uniform_cover(sph,nump)
        #check that the centroid is still the center
        sampled_centroid = IMP.algebra.Vector3D(0.0,0.0,0.0)
        self.assert_(len(points)>=nump)
        print "the sample"
        for i in range(len(points)):
            sampled_centroid = sampled_centroid + points[i]
            print ".dotat " + str(points[i][0]) + " "\
                + str(points[i][1]) + "  " + str(points[i][2])
        sampled_centroid = sampled_centroid * (1.0/len(points))
        sampled_centroid.show()
        center.show()
        self.assertInTolerance((sampled_centroid-center).get_magnitude(), 0, 1.0)


if __name__ == '__main__':
    unittest.main()
