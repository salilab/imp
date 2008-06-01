import unittest
import IMP
import IMP.utils
import IMP.test


class RefCountTests(IMP.test.TestCase):
    """Test refcounting of particles"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)
        self.basenum= IMP.RefCountedObject.get_number_of_live_objects()
        print "The base number of objects is " + str(self.basenum)

    def _check_number(self, expected):
        print "Expected "+str(expected)\
            + " got " + str(IMP.RefCountedObject.get_number_of_live_objects()
                            - self.basenum)
        self.assertEqual(IMP.RefCountedObject.get_number_of_live_objects() - self.basenum,
                         expected,
                         "wrong number of particles")

    def __test_simple(self):
        """Check that ref counting of particles works within python"""
        # swig is broken so this needs to be skipped
        self._check_number(0)

        p= IMP.Particle()
        p=1

        self._check_number(0)

        m= IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        self._check_number(1)

        p=1
        self._check_number(1)
        m=1
        self._check_number(0)

    def test_removal(self):
        """Check that ref counting works with removing particles"""
        self._check_number(0)
        m= IMP.Model()
        p= IMP.Particle()
        pi= m.add_particle(p)
        self._check_number(1)
        m.remove_particle(pi)
        self._check_number(1)
        self.assert_(not p.get_is_active(), "Removed particle is still active")
        p=1
        self._check_number(0)

    # This test does not work since swig refcounting is broken
    def __test_round_trip(self):
        """test that tracking survives the round trip"""

        print "test that the round trip object doesn't delete it"
        self._check_number(0)
        m= IMP.Model()
        print 1
        p= IMP.Particle()
        print p
        print 2
        pi= m.add_particle(p)
        print 2.5
        p=1
        self._check_number(1)
        print "got particle back"
        print 3
        p= m.get_particle(pi)
        print str(p)
        self._check_number(1)
        print 4
        p=None
        self._check_number(1)
        m=1
        print 6

        print "test that the round trip object keeps it alive"
        self._check_number(0)
        m= IMP.Model()
        print 7
        p= IMP.Particle()
        print p
        print 8
        pi= m.add_particle(p)
        print 9
        p=None
        self._check_number(1)
        print "got particle back"
        print 10
        p= m.get_particle(pi)
        print p
        self._check_number(1)
        print "removing particle"
        print 11
        p=None
        print 11.25
        p= m.get_particle(pi)
        print 11.5
        m.remove_particle(pi)
        self._check_number(1)
        print 12
        p=None
        self._check_number(0)


    def test_shared(self):
        """Check that ref counting works with shared particles"""
        print "max change"
        self._check_number(0)
        m= IMP.Model()
        p= IMP.Particle()
        print "Add particle"
        pi= m.add_particle(p)
        d= IMP.XYZDecorator.create(p)
        d=0
        mc= IMP.MaxChangeScoreState(IMP.XYZDecorator.get_xyz_keys())
        print "Add particle to mc"
        mc.add_particle(p)
        # also have the score state now
        self._check_number(2)
        print "Remove from model"
        m.remove_particle(pi)
        self._check_number(2)
        p=1
        self._check_number(2)
        print "Remove from mc"
        mc.clear_particles()
        self._check_number(1)
        mc=0
        self._check_number(0)

    def test_skip(self):
        """Check that removed particles are skipped"""
        print "skipped"
        m= IMP.Model()
        p= IMP.Particle()
        pi= m.add_particle(p)
        ps= m.get_particles()
        self.assertEqual(len(ps), 1, "Should only be 1 particle")
        m.remove_particle(pi)
        ps= m.get_particles()
        self.assertEqual(len(ps), 0, "Should no particles particle")

    def test_restraints(self):
        m= IMP.Model()
        r= IMP.ConstantRestraint(1)
        s= IMP.RestraintSet()
        m.add_restraint(s)
        m.add_restraint(r)
        s.add_restraint(r)
        m.evaluate(False)
        r=0
        s=0
        m=0
        self._check_number(0)



if __name__ == '__main__':
    unittest.main()
