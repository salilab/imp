import IMP
import unittest
import IMP.utils
import IMP.test
import IMP.core
import IMP.algebra

class DistanceTests(IMP.test.TestCase):
    """Test the BindBondPairScore"""
    def test_constructions(self):
        """Test the cylinder edge cases"""
        V= IMP.algebra.Vector3D
        S= IMP.core.Segment

        v=V(0,0,0)
        s=S(V(-1,1,1), V(1,1,1))
        ss= IMP.core.shortest_segment(s,v)
        ss.first.show()
        ss.second.show()
        self.assertInTolerance((ss.first-V(0, 1, 1)).get_magnitude(), 0, .1)
        self.assertInTolerance((ss.second-V(0, 0, 0)).get_magnitude(), 0, .1)
        #self.assertEqual(

        s=S(V(.1,1,1), V(1,1,1))
        ss= IMP.core.shortest_segment(s,v)
        ss.first.show()
        ss.second.show()
        self.assertInTolerance((ss.first-V(0.1, 1, 1)).get_magnitude(), 0, .1)
        self.assertInTolerance((ss.second-V(0, 0, 0)).get_magnitude(), 0, .1)
        print "last"
        s=S(V(0,0,0), V(1,1,1))
        s2=S(V(1,0,0), V(0,1,1))
        ss= IMP.core.shortest_segment(s, s2)
        print "show last"
        ss.first.show()
        ss.second.show()
        self.assertInTolerance((ss.first-V(0.5, 0.5, .5)).get_magnitude(), 0, .1)
        self.assertInTolerance((ss.second-V(0.5, 0.5, 0.5)).get_magnitude(), 0, .1)


    def test_random(self):
        """Test random cylinder/cylinder vectors"""
        V= IMP.algebra.Vector3D
        S= IMP.core.Segment
        IMP.set_log_level(IMP.VERBOSE)
        m=IMP.Model()
        p00= IMP.Particle()
        m.add_particle(p00)
        xyz00= IMP.core.XYZDecorator.create(p00)
        b00= IMP.core.BondedDecorator.create(p00)
        p01= IMP.Particle()
        m.add_particle(p01)
        xyz01= IMP.core.XYZDecorator.create(p01)
        b01= IMP.core.BondedDecorator.create(p01)
        b0= IMP.core.custom_bond(b00, b01, 1)
        p10= IMP.Particle()
        m.add_particle(p10)
        xyz10= IMP.core.XYZDecorator.create(p10)
        b10= IMP.core.BondedDecorator.create(p10)
        p11= IMP.Particle()
        m.add_particle(p11)
        xyz11= IMP.core.XYZDecorator.create(p11)
        b11= IMP.core.BondedDecorator.create(p11)
        b1= IMP.core.custom_bond(b10, b11, 1)
        l= IMP.core.Linear(0,1)
        ps= IMP.core.BondBondPairScore(l)
        for i in range(0,1):
            s0= S(IMP.algebra.random_vector_in_box(V(0,0,0), V(100,100,100)),
                  IMP.algebra.random_vector_in_box(V(0,0,0), V(100,100,100)))
            s1= S(IMP.algebra.random_vector_in_box(V(0,0,0), V(100,100,100)),
                  IMP.algebra.random_vector_in_box(V(0,0,0), V(100,100,100)))
            ss= IMP.core.shortest_segment(s0, s1)
            v0= s0.second - s0.first
            v1= s1.second - s1.first
            vs= ss.second - ss.first
            s0.first.show(); s0.second.show();print
            s1.first.show(); s1.second.show(); print
            ss.first.show(); ss.second.show(); print
            v0.show(); print
            v1.show(); print
            vs.show(); print
            if (s0.first- ss.first).get_magnitude() > .1 \
                    and (s0.second- ss.first).get_magnitude() > .1 \
                    and (s1.first- ss.second).get_magnitude() > .1 \
                    and (s1.second- ss.second).get_magnitude() > .1:
                self.assertInTolerance(v0*vs, 0, .1)
                self.assertInTolerance(v1*vs, 0, .1)
            xyz00.set_coordinates(s0.first)
            xyz01.set_coordinates(s0.second)
            xyz10.set_coordinates(s1.first)
            xyz11.set_coordinates(s1.second)
            v=ps.evaluate(b0.get_particle(), b1.get_particle(), None)
            self.assertInTolerance(v, (ss.first-ss.second).get_magnitude(), .1)

if __name__ == '__main__':
    unittest.main()
