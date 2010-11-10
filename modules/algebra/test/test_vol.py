import IMP
import IMP.test
import IMP.algebra
import math

class SphereTests(IMP.test.TestCase):
    def test_enc(self):
        """Check volume and surface area of random"""
        if not IMP.algebra.has_cgal:
            return
        ss= IMP.algebra.Sphere3Ds()
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
        self.assert_(sp[0] > 6*25*25)
        self.assert_(sp[0] < 6*30*30)
        self.assert_(sp[1] < 30*30*30)
        self.assert_(sp[1] > 25*25*25)

if __name__ == '__main__':
    IMP.test.main()
