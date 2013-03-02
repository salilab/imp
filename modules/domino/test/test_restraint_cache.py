import IMP
import IMP.test
import IMP.domino
import IMP.core
import random

key= IMP.IntKey("assignment")

class Tests(IMP.test.TestCase):
    def _create_stuff(self):
        m= IMP.Model()
        ps =[IMP.Particle(m) for i in range(0,10)]
        r= IMP.kernel._ConstRestraint(1, ps)
        r.set_model(m)
        r.set_name("const restraint")
        pst= IMP.domino.ParticleStatesTable()
        ik=IMP.IntKey("key")
        for p in ps:
            p.add_attribute(ik, 0)
            pst.set_particle_states(p, IMP.domino.IndexStates(4, ik))
        cache= IMP.domino.RestraintCache(pst, 4)
        return (m, ps, r, pst, ik, cache)
    def test_decomposition(self):
        """Test cache with decomposition with no max is empty"""
        (m, ps, r, pst, ik, cache)=self._create_stuff()
        cache.add_restraints([r])
        subset=IMP.domino.Subset(random.sample(ps, 2))
        print subset
        rsb= cache.get_restraints(subset,[])
        self.assertEqual(len(rsb), 0)

    def test_decomposition_2(self):
        """Test cache with decomposition with with a max is non-empty"""
        (m, ps, r, pst, ik, cache)=self._create_stuff()
        r.set_maximum_score(2)
        r.set_weight(3)
        cache.add_restraints([r])
        subset=IMP.domino.Subset(random.sample(ps, 2))
        print subset
        rsb= cache.get_restraints(subset,[])
        print rsb
        self.assertEqual(len(rsb), 2)
        slc= cache.get_slice(rsb[0], subset)
        print slc
        self.assertEqual(len(slc), 1)
        aslc= slc.get_sliced(IMP.domino.Assignment([0,1]))
        print aslc
        self.assertEqual(len(aslc), 1)
        score= cache.get_score(rsb[0], aslc)
        self.assertAlmostEqual(score, 1.0/len(ps), delta=.01)

    def test_decomposition_3(self):
        """Test cache with decomposition with set exclusion"""
        (m, ps, r, pst, ik, cache)=self._create_stuff()
        r.set_maximum_score(2)
        r.set_weight(3)
        cache.add_restraints([r])
        subset=IMP.domino.Subset(ps)
        print subset
        rsb= cache.get_restraints(subset,[IMP.domino.Subset([p]) for p in ps])
        print rsb
        self.assertEqual(len(rsb), 1)
        slc= cache.get_slice(rsb[0], subset)
        print slc
        self.assertEqual(len(slc), len(ps))
        aslc= slc.get_sliced(IMP.domino.Assignment(range(len(ps))))
        print aslc
        self.assertEqual(len(aslc), len(ps))
        score= cache.get_score(rsb[0], aslc)
        print score
        self.assert_(score>1000)

    def test_decomposition_4(self):
        """Test cache with decomposition with nested sets"""
        (m, ps, r, pst, ik, cache)=self._create_stuff()
        rs= IMP.RestraintSet("outer")
        rs.set_model(m)
        rs.set_maximum_score(2)
        rs.add_restraint(r)
        r.set_weight(3)
        cache.add_restraints([rs])
        subset=IMP.domino.Subset(ps)
        rsb= cache.get_restraints(subset,[IMP.domino.Subset([p]) for p in ps])
        self.assertEqual(len(rsb), 1)
        slc= cache.get_slice(rsb[0], subset)
        self.assertEqual(len(slc), len(ps))
        aslc= slc.get_sliced(IMP.domino.Assignment(range(len(ps))))
        self.assertEqual(len(aslc), len(ps))
        score= cache.get_score(rsb[0], aslc)
        self.assert_(score>1000)

    def test_decomposition_5(self):
        """Test cache with simple restraint"""
        (m, ps, r, pst, ik, cache)=self._create_stuff()
        rs= IMP.RestraintSet("outer")
        rs.set_model(m)
        r= IMP.kernel._ConstRestraint(1, [ps[0]])
        r.set_name("const 2")
        rs.add_restraint(r)
        r.set_maximum_score(.5)
        cache.add_restraints([rs])
        print "info:"
        cache.show_restraint_information()
        subset=IMP.domino.Subset([ps[0]])
        rsb= cache.get_restraints(subset,[])
        self.assertEqual(len(rsb), 1)
        slc= cache.get_slice(rsb[0], subset)
        self.assertEqual(len(slc), 1)
        aslc= slc.get_sliced(IMP.domino.Assignment(range(len(ps))))
        self.assertEqual(len(aslc), 1)
        score= cache.get_score(rsb[0], aslc)
        print score
        self.assert_(score>1000)

if __name__ == '__main__':
    IMP.test.main()
