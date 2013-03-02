import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_bonded(self):
        """Check radius of gyration """
        m = IMP.Model()
        bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(20,20,20),
                                     IMP.algebra.Vector3D(120, 120, 120))
        ps = []
        vs = []
        for i in range(0,400):
            p = IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 0))
            md= IMP.atom.Mass.setup_particle(p, 1)
            ps.append(p)
            vs.append(d.get_coordinates())
        rg0= IMP.algebra.get_radius_of_gyration(vs)
        rg1= IMP.atom.get_radius_of_gyration(ps)
        self.assertAlmostEqual(rg0, rg1, delta=1)

    def test_ball(self):
        """Check radius of gyration of a ball"""
        m = IMP.Model()
        bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(20,20,20),
                                     IMP.algebra.Vector3D(120, 120, 120))
        s= IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 20)
        print s
        p= IMP.Particle(m)
        IMP.core.XYZR.setup_particle(p, s)
        ps=[p]
        vs=[]
        for i in range(0,400):
            vs.append(IMP.algebra.get_random_vector_in(s))
        rg0= IMP.algebra.get_radius_of_gyration(vs)
        rg1= IMP.atom.get_radius_of_gyration(ps)
        self.assertAlmostEqual(rg0, rg1, delta=1)
    def test_ball2(self):
        """Check radius of gyration with two balls"""
        m = IMP.Model()
        bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(20,20,20),
                                     IMP.algebra.Vector3D(120, 120, 120))
        s0= IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 20)
        s1= IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 30)
        print s0, s1
        p0= IMP.Particle(m)
        IMP.core.XYZR.setup_particle(p0, s0)
        p1= IMP.Particle(m)
        IMP.core.XYZR.setup_particle(p1, s1)
        ps=[p0,p1]
        vs=[]
        for i in range(0,20**3):
            vs.append(IMP.algebra.get_random_vector_in(s0))
        for i in range(0,30**3):
            vs.append(IMP.algebra.get_random_vector_in(s1))
        rg0= IMP.algebra.get_radius_of_gyration(vs)
        rg1= IMP.atom.get_radius_of_gyration(ps)
        self.assertAlmostEqual(rg0, rg1, delta=1)

if __name__ == '__main__':
    IMP.test.main()
