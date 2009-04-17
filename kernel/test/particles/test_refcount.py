import unittest
import IMP
import IMP.utils
import IMP.test

# We probably shouldn't require IMP.core to test the kernel; temporary hack
import IMP.core

class RefCountTests(IMP.test.TestCase):
    """Test refcounting of particles"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)
        self.basenum= IMP.RefCounted.get_number_of_live_objects()

    def _check_number(self, expected):
        self.assertEqual(IMP.RefCounted.get_number_of_live_objects() \
                         - self.basenum, expected)

    def test_simple(self):
        """Check that ref counting of particles works within python"""
        m= IMP.Model()
        self._check_number(1)
        p= IMP.Particle(m)
        self._check_number(2)
        del p
        self._check_number(2)
        del m
        # Deleting Model should delete all Particles too
        self._check_number(0)

    def test_removal(self):
        """Check that ref counting works with removing particles"""
        m= IMP.Model()
        p= IMP.Particle(m)
        self._check_number(2)
        m.remove_particle(p)
        # Particle should not disappear yet since Python still has a reference
        self._check_number(2)
        self.assert_(not p.get_is_active(), "Removed particle is still active")
        del p
        self._check_number(1)
        del m
        self._check_number(0)

    def test_delete_model_constructor(self):
        """Constructed Python Particles should survive model deletion"""
        m= IMP.Model()
        p= IMP.Particle(m)
        self.assertEqual(p.get_ref_count(), 2)
        self._check_number(2)
        # New particle p should not go away until we free the Python reference
        del m
        self._check_number(1)
        self.assertEqual(p.get_ref_count(), 1)
        del p
        self._check_number(0)

    def test_delete_model_iterator(self):
        """Python Particles from iterators should survive model deletion"""
        m= IMP.Model()
        IMP.Particle(m)
        # Now create new Python particle p from C++ iterator
        # (not the Python IMP.Particle() constructor)
        # This calls swig::from() internally, which is modified by template
        # specialization in our SWIG interface.
        p = m.get_particles().__iter__().value()
        # Python reference p plus C++ reference from m
        self.assertEqual(p.get_ref_count(), 2)
        del m
        # Now only the Python reference p should survive
        self.assertEqual(p.get_ref_count(), 1)
        self._check_number(1)
        del p
        self._check_number(0)

    def test_delete_model_accessor(self):
        "Python Particles from vector accessors should survive model deletion"
        m= IMP.Model()
        IMP.Particle(m)
        # Now create new Python particle p from a C++ vector accessor
        # (front(), back(), [], etc.)
        # (not the Python IMP.Particle() constructor)
        # These accessors call specific methods in the SWIG wrapper which
        # are modified by typemaps in our interface.
        p = m.get_particles()[0]
        # Python reference p plus C++ reference from m
        self.assertEqual(p.get_ref_count(), 2)
        del m
        # Now only the Python reference p should survive
        self.assertEqual(p.get_ref_count(), 1)
        self._check_number(1)
        del p
        self._check_number(0)

    def test_shared(self):
        """Check that ref counting works with shared particles"""
        m= IMP.Model()
        p= IMP.Particle(m)
        d= IMP.core.XYZDecorator.create(p)
        del d

        mc= IMP.core.ListSingletonContainer()
        mc.add_particle(p)
        # also have the score state now
        self._check_number(3)
        m.remove_particle(p)
        self.assertEqual(m.get_number_of_particles(), 0)
        self._check_number(3)
        del p
        self._check_number(3)
        mc.clear_particles()
        self.assertEqual(mc.get_number_of_particles(), 0)
        self._check_number(2)
        del mc
        self._check_number(1)

    def test_skip(self):
        """Check that removed particles are skipped"""
        m= IMP.Model()
        p= IMP.Particle(m)
        ps= m.get_particles()
        self.assertEqual(len(ps), 1, "Should only be 1 particle")
        m.remove_particle(p)
        ps= m.get_particles()
        self.assertEqual(len(ps), 0, "Should be no particles")

    def test_restraints(self):
        """Check reference counting of restraints"""
        m= IMP.Model()
        r= IMP.core.ConstantRestraint(1)
        s= IMP.core.RestraintSet()
        m.add_restraint(s)
        m.add_restraint(r)
        s.add_restraint(r)
        m.evaluate(False)
        self._check_number(3)
        # Model should hold a ref to restraints, so nothing should be freed
        # until it is
        del r
        self._check_number(3)
        del s
        self._check_number(3)
        del m
        self._check_number(0)


if __name__ == '__main__':
    unittest.main()
