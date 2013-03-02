import IMP
import IMP.test
import IMP.container
import IMP.display
import math

class Tests(IMP.test.TestCase):
    """Tests for tunnel scores"""
    def test_connectivity_zero_set(self):
        """Test consecutive pair container decomposition"""
        m= IMP.Model()
        ps=[IMP.Particle(m) for i in range(0,15)]
        ds=[IMP.core.XYZR.setup_particle(p) for p in ps]
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        for d in ds:
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            print d
            d.set_radius(4)
        cpc= IMP.container.ConsecutivePairContainer(ps)
        hdps= IMP.core.HarmonicDistancePairScore(0,1)
        r= IMP.container.PairsRestraint(hdps, cpc)
        m.add_restraint(r)
        self.assert_(r.evaluate(False) > 0)
        rd= r.create_decomposition()
        rds= IMP.get_restraints([rd])
        self.assertEqual(len(rds), len(ps)-1)
        re= r.evaluate(False)
        rde= rd.evaluate(False)
        rdse= [x.evaluate(False) for x in rds]
        self.assertAlmostEqual(re, rde, delta=.1)
        self.assertAlmostEqual(re, sum(rdse), delta=.1)
        print re, rde, sum(rdse)

    def test_pair_filter(self):
        """Test consecutive pair filter"""
        m= IMP.Model()
        ps=[IMP.Particle(m) for i in range(0,15)]
        ecpc= IMP.container.ExclusiveConsecutivePairContainer(ps)
        ef= IMP.container.ExclusiveConsecutivePairFilter();
        for i in range(1,len(ps)):
            self.assertEquals(ef.get_value((ps[i-1], ps[i])), 1)
        for i in range(2,len(ps)):
            self.assertEquals(ef.get_value((ps[i-2], ps[i])), 0)




if __name__ == '__main__':
    IMP.test.main()
