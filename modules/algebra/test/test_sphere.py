import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

    def test_sphere_construction(self):
        """Check that sphere are constructed correctly"""
        center = IMP.algebra.Vector3D(0.0,0.0,0.0)
        radius=5.0
        sph = IMP.algebra.Sphere3D(center,radius)
        self.assertEqual((sph.get_center()-center).get_magnitude() < 0.01,True)
        self.assertEqual(sph.get_radius(),radius)
        self.assertAlmostEqual(IMP.algebra.get_surface_area(sph),math.pi*25.0*4,
                               places=1)
        self.assertAlmostEqual(IMP.algebra.get_volume(sph),math.pi*125.0*(4.0/3),
                               places=1)

    def test_enclosing(self):
        """Check enclosing sphere"""
        ss= []
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        for i in range(0,20):
            v= IMP.algebra.get_random_vector_in(bb)
            r= i
            s= IMP.algebra.Sphere3D(v,r)
            ss.append(s)
        for i in range(1,20):
            css=ss[0:i]
            print len(css)
            es= IMP.algebra.get_enclosing_sphere(css)
            es.show(); print
            for cs in css:
                d= (cs.get_center()-es.get_center()).get_magnitude()
                cs.show(); print
                self.assertLess(d+ cs.get_radius()- es.get_radius(), .5)

    def test_sampling_in_sphere( self ):
        """Verify uniform sampling within a sphere"""
        s = IMP.algebra.Sphere2D( [0,0], 1 )
        inner_box = \
            IMP.algebra.BoundingBox2D( [-0.5, -0.5], [0.5, 0.5] )
        n = 0
        m = 2000
        for i in range(m):
            v = IMP.algebra.get_random_vector_in( s )
            if inner_box.get_contains(v):
                n=n+1
        # compare expected and observed hits
        s_area = math.pi # pi * 1.0^2
        inner_box_area = 1.0
        expected_p = inner_box_area / s_area
        observed_p = float(n) / float(m)
        print expected_p, observed_p
        self.assertAlmostEqual(observed_p, expected_p, places=1)


if __name__ == '__main__':
    IMP.test.main()
