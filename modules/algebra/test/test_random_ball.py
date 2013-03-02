import IMP.test
import IMP.algebra
#import IMP.display
import math
import IMP.statistics

class Tests(IMP.test.TestCase):
    def test_circle(self):
        """Test the creation of points on a circle"""
        s2= IMP.algebra.Sphere2D(IMP.algebra.Vector2D(.5,.5), 1.5)
        h= IMP.statistics.Histogram2D(.1, IMP.algebra.get_bounding_box(s2))
        #w= IMP.display.PymolWriter("pts2d.pym")
        for i in range(0,10000):
            pt= IMP.algebra.get_random_vector_on(s2)
            pt3= IMP.algebra.Vector3D(pt[0], pt[1], 0)
            #g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(pt3, .01))
            #w.add_geometry(g)
            h.add(pt)
        center= h.get_mean()
        std= h.get_standard_deviation(center)
        print center, std
        for i in range(0,2):
            self.assertAlmostEqual(center[i], .5, delta=.05)
            self.assertAlmostEqual(std[i], .7*1.5, delta=.05)
    def test_sphere(self):
        """Test the creation of points on a sphere"""
        s2= IMP.algebra.Sphere3D(IMP.algebra.Vector3D(.75,.75,.75), .7)
        h= IMP.statistics.Histogram3D(.1, IMP.algebra.get_bounding_box(s2))
        #w= IMP.display.PymolWriter("pts2d.pym")
        for i in range(0,10000):
            pt= IMP.algebra.get_random_vector_on(s2)
            #g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(pt, .01))
            #w.add_geometry(g)
            h.add(pt)
        center= h.get_mean()
        std= h.get_standard_deviation(center)
        print center, std
        for i in range(0,3):
            self.assertAlmostEqual(center[i], .75, delta=.016)
            self.assertAlmostEqual(std[i], .58*.7, delta=.05)

if __name__ == '__main__':
    IMP.test.main()
