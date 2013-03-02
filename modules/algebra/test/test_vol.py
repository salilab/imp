import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):
    def test_enc(self):
        """Check volume and surface area of random"""
        if not IMP.algebra.IMP_ALGEBRA_HAS_IMP_CGAL:
            self.skipTest("IMP.cgal module disabled")
        ss= []
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        for i in range(0,100):
            v= IMP.algebra.get_random_vector_on(bb)
            r= 10
            s= IMP.algebra.Sphere3D(v,r)
            ss.append(s)
        for x in [0,10]:
            for y in [0,10]:
                for z in [0,10]:
                    s= IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,y,z), 10)
                    ss.append(s)
        for s in ss:
            print ".sphere", s.get_center()[0], s.get_center()[1], s.get_center()[2], s.get_radius()
        print 30*30*30
        sp= IMP.algebra.get_surface_area_and_volume(ss)
        print sp
        print 6*25*25, 6*30*30, 30*30*30, 25*25*25
        self.assertGreater(sp[0], .9*6*25*25)
        self.assertLess(sp[0], 6*30*30)
        self.assertLess(sp[1], 30*30*30)
        self.assertGreater(sp[1], 25*25*25)

if __name__ == '__main__':
    IMP.test.main()
