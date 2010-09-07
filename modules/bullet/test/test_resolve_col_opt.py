import unittest
import IMP
import IMP.test
import IMP.bullet
import IMP.core
import IMP.algebra
import StringIO
import math

class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def count_hits(self, ps):
        ret=0;
        for i in range(0, len(ps)):
            for j in range(0,i):
                pi= ps[i]
                pj= ps[j]
                di= IMP.core.XYZR(pi)
                dj= IMP.core.XYZR(pj)
                if IMP.core.get_distance(di, dj) < 0:
                    ret=ret+1
        return ret
    def test_rco(self):
        """Test basic ResolveCollision optimization"""
        m= IMP.Model()
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(5,5,5))
        ps=[]
        for i in range(0,100):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)
            ps.append(p)
            d.set_radius(2)
            print d
        print "intesections: ", self.count_hits(ps)
        opt= IMP.bullet.ResolveCollisionsOptimizer(m.get_root_restraint_set(), ps)
        for i in range(0,10):
            opt.optimize(100);
            print "intesections: ", self.count_hits(ps)
    def test_rcos(self):
        """Test basic ResolveCollision optimization with springs"""
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(5,5,5))
        ps=[]
        for i in range(0,100):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)
            ps.append(p)
            d.set_radius(2)
            print d
        for i in range(1,6):
            r= IMP.core.PairRestraint(IMP.core.HarmonicDistancePairScore(4,1),
                                      (ps[i-1], ps[i]));
            m.add_restraint(r)
        print "intesections:", self.count_hits(ps),"score:", m.evaluate(False)
        opt= IMP.bullet.ResolveCollisionsOptimizer(m.get_root_restraint_set(), ps)
        for i in range(0,10):
            opt.optimize(100);
            print "intesections:", self.count_hits(ps), "score:", m.evaluate(False)

if __name__ == '__main__':
    unittest.main()
