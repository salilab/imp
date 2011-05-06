import IMP
import IMP.test
import IMP.core

class GetFromTests(IMP.test.TestCase):
    def test_get_from_null(self):
        """Test get_from with NULL pointer"""
        self.assertRaises(ValueError, IMP.core.TripletRestraint.get_from, None)

    def test_cast(self):
        """Test get_from cast"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        p3 = IMP.Particle(m)
        for p in [p1,p2,p3]:
            IMP.core.XYZ.setup_particle(p)
        ar = IMP.core.AngleRestraint(IMP.core.Harmonic(0., 1.), p1, p2, p3)
        ar.set_name("test restraint")
        ar.set_was_used(True)

        # Test cast to base class
        tr = IMP.core.TripletRestraint.get_from(ar)
        self.assertEqual(tr.get_name(), "test restraint")
        self.assert_(isinstance(tr, IMP.core.TripletRestraint))

        # Test cast to same class or to derived class
        for r in [tr, ar]:
            ar2 = IMP.core.AngleRestraint.get_from(r)
            self.assert_(isinstance(ar2, IMP.core.AngleRestraint))
            self.assertEqual(ar, ar2)

        # Cast to unrelated class should fail
        self.assertRaises(ValueError, IMP.core.ConstantRestraint.get_from, ar)

        ts = IMP.core.AngleTripletScore(IMP.core.Harmonic(0., 1.))
        tr = IMP.core.TripletRestraint(ts, [p1, p2, p3])
        tr.set_was_used(True)
        # Cannot cast to derived class if it is not really of that type
        self.assertRaises(ValueError, IMP.core.AngleRestraint.get_from, tr)

if __name__ == '__main__':
    IMP.test.main()
