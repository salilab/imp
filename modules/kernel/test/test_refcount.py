import IMP
import IMP.test
import sys

class Tests(IMP.test.TestCase):
    """Test refcounting of particles"""

    def test_simple(self):
        """Check that ref counting of particles works within python"""
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("ref counting particles")
        refcnt.assert_number(1)
        p= IMP.Particle(m)
        refcnt.assert_number(2)
        del p
        refcnt.assert_number(2)
        del m
        # Deleting Model should delete all Particles too
        refcnt.assert_number(0)


    def test_delete_model_accessor(self):
        "Python Particles from vector accessors should survive model deletion"
        IMP.base.set_log_level(IMP.MEMORY)
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("test model")
        IMP.Particle(m)
        # Now create new Python particle p from a C++ vector accessor
        # (front(), back(), [], etc.)
        # (not the Python IMP.Particle() constructor)
        # These accessors call specific methods in the SWIG wrapper which
        # are modified by typemaps in our interface.
        print "getting particles"
        ps=m.get_particles()
        print "getting particle"
        p = ps[0]
        del ps
        # Python reference p plus C++ reference from m
        self.assertEqual(p.get_ref_count(), 2)
        print "deleting model"
        del m
        # Now only the Python reference p should survive
        self.assertEqual(p.get_ref_count(), 1)
        refcnt.assert_number(1)
        print p
        print "deleting p"
        print p.thisown
        del p
        refcnt.assert_number(0)

    def test_delete_model_accessor_restraint(self):
        "Python restraints from vector accessors should survive model deletion"
        IMP.base.set_log_level(IMP.MEMORY)
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("test model")
        r=IMP.kernel._ConstRestraint(1)
        print "adding"
        m.add_restraint(r)
        print r
        print r.__del__
        # Now create new Python particle p from a C++ vector accessor
        # (front(), back(), [], etc.)
        # (not the Python IMP.Particle() constructor)
        # These accessors call specific methods in the SWIG wrapper which
        # are modified by typemaps in our interface.
        print "getting restraints"
        ps=m.get_restraints()
        print "getting restraint"
        p = ps[0]
        print sys.getrefcount(p), len(ps)
        print p
        # r, m, p/ps
        self.assertEqual(p.get_ref_count(), 3)
        print "deleting ps"
        del ps
        # r, m, p
        self.assertEqual(p.get_ref_count(), 3)
        print "getting it again"
        rp= m.get_restraint(0)
        print "check"
        # Python reference p, m, r, rp
        self.assertEqual(p.get_ref_count(), 4)
        print p
        print p.thisown
        print dir(p)
        print sys.getrefcount(p)
        print "deleting p"
        #del p
        #p.__del__
        print p.__del__
        del p
        # Python reference m, r, rp
        print "checking p"
        self.assertEqual(r.get_ref_count(), 3)
        print 'deleting rp'
        del rp
        # Python reference m, r
        self.assertEqual(r.get_ref_count(), 2)
        print "deleting model"
        del m
        refcnt.assert_number(1)
        # Now only the Python reference p should survive
        self.assertEqual(r.get_ref_count(), 1)
        print 'deleting r'
        print r
        del r
        refcnt.assert_number(0)

    def test_delete_model_accessor_2(self):
        "Python Restraints from vector accessors should survive model deletion"
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("restraint survival")
        r = IMP.RestraintSet()
        m.add_restraint(r)
        del r
        # Now create new Python Restraint r from a C++ vector accessor
        # These accessors call specific methods in the SWIG wrapper which
        # are modified by typemaps in our interface.
        r = m.get_restraints()[0]
        # Python reference r plus C++ reference from m
        self.assertEqual(r.get_ref_count(), 2)
        del m
        # Now only the Python reference r should survive
        self.assertEqual(r.get_ref_count(), 1)
        refcnt.assert_number(1)
        del r
        refcnt.assert_number(0)

    def test_simple_rest(self):
        """Check reference counting of restraints"""
        refcnt = IMP.test.RefCountChecker(self)
        IMP.base.set_log_level(IMP.base.MEMORY)
        m= IMP.Model("M")
        r= IMP.kernel._ConstRestraint(1)
        r.set_name("R")
        s= IMP.RestraintSet("S")
        print "add s to m"
        m.add_restraint(s)
        print "add r to m"
        m.add_restraint(r)
        print "add r to s"
        s.add_restraint(r)
        """m.evaluate(False)
        refcnt.assert_number(3)
        # Model should hold a ref to restraints, so nothing should be freed
        # until it is
"""
        print r.get_ref_count(), s.get_ref_count(), m.get_ref_count()
        del r
        refcnt.assert_number(3)
        print s.get_ref_count(), m.get_ref_count()
        del s
        refcnt.assert_number(3)
        print m.get_ref_count()
        del m
        print "check"
        refcnt.assert_number(0)

    def test_delete_model_iterator(self):
        """Python Restraints from iterators should survive model deletion"""
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("python restraint survival")
        r = IMP.RestraintSet()
        m.add_restraint(r)
        del r
        # Now create new Python Restraint r from C++ iterator
        # This calls swig::from() internally, which is modified by template
        # specialization in our SWIG interface.
        r = m.get_restraints()[0]
        # Python reference r plus C++ reference from m
        self.assertEqual(r.get_ref_count(), 2)
        del m
        # Now only the Python reference r should survive
        self.assertEqual(r.get_ref_count(), 1)
        refcnt.assert_number(1)
        del r
        refcnt.assert_number(0)

    def test_removal(self):
        """Check that ref counting works with removing particles"""
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("ref counting and particle removal")
        p= IMP.Particle(m)
        refcnt.assert_number(2)
        m.remove_particle(p)
        # Particle should not disappear yet since Python still has a reference
        refcnt.assert_number(2)
        self.assertFalse(p.get_is_active(), "Removed particle is still active")
        del p
        refcnt.assert_number(1)
        del m
        refcnt.assert_number(0)

    def test_delete_model_constructor(self):
        """Constructed Python Particles should survive model deletion"""
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("python restraint survival")
        p= IMP.Particle(m)
        self.assertEqual(p.get_ref_count(), 2)
        refcnt.assert_number(2)
        # New particle p should not go away until we free the Python reference
        del m
        refcnt.assert_number(1)
        self.assertEqual(p.get_ref_count(), 1)
        del p
        refcnt.assert_number(0)


    def test_skip(self):
        """Check that removed particles are skipped"""
        m= IMP.Model("skipping removed particles")
        p= IMP.Particle(m)
        ps= m.get_particles()
        self.assertEqual(len(ps), 1, "Should only be 1 particle")
        m.remove_particle(p)
        ps= m.get_particles()
        self.assertEqual(len(ps), 0, "Should be no particles")


    def test_sequence(self):
        """Test that sequences are refcounted properly"""
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model("sequence ref counting")
        p= IMP.Particle(m)
        ps= m.get_particles()
        print IMP.kernel._take_particles(ps)
        del m
        del ps
        del p
        refcnt.assert_number(0)


if __name__ == '__main__':
    IMP.test.main()
