import unittest
import IMP
import IMP.test

# We probably shouldn't require IMP.core to test the kernel; temporary hack
import IMP.core

class RefCountTests(IMP.test.TestCase):
    """Test refcounting of particles"""
    def _test_refcount_container(self):
        """Check that Particles is ref counted"""
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model()
        p= IMP.Particle(m)
        ps= IMP.Particles()
        refcnt.assert_number(2)
        ps.append(p)
        m.remove_particle(p)
        refcnt.assert_number(2)
        del p
        refcnt.assert_number(2)
        del ps
        refcnt.assert_number(1)
        del m
        refcnt.assert_number(0)

    def test_refcount_container(self):
        """Check that ParticlePairs is ref counted"""
        refcnt = IMP.test.RefCountChecker(self)
        IMP.set_log_level(IMP.MEMORY)
        m= IMP.Model()
        p= IMP.Particle(m)
        ps= IMP.ParticlePairs()
        refcnt.assert_number(2)
        print "pp"
        ps.append(IMP.ParticlePair(p,p))
        print 0
        m.remove_particle(p)
        refcnt.assert_number(2)
        print 1
        del p
        refcnt.assert_number(2)
        print 2
        del ps
        refcnt.assert_number(1)
        print 3
        del m
        refcnt.assert_number(0)
        IMP.set_log_level(IMP.SILENT)


if __name__ == '__main__':
    unittest.main()
