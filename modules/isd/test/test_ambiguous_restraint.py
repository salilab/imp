import IMP
import IMP.algebra
import IMP.core
import IMP.isd
import IMP.test
import pickle


def _make_test_restraint():
    m = IMP.Model()
    p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                     IMP.algebra.Vector3D(0,0,0))
    p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                     IMP.algebra.Vector3D(3,3,3))
    p3 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                     IMP.algebra.Vector3D(9,9,9))
    d1 = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(1, 1), p1, p2)
    d2 = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(1, 1), p2, p3)
    r = IMP.isd.AmbiguousRestraint(m, -10, d1, d2)
    return m, r


class Tests(IMP.test.TestCase):

    def test_simple(self):
        """Simple test of AmbiguousRestraint"""
        m, r = _make_test_restraint()
        self.assertAlmostEqual(r.evaluate(True), 8.8038, delta=0.001)

    def test_serialize(self):
        """Test (un-)serialize of AmbiguousRestraint"""
        m, r = _make_test_restraint()
        r.set_name("foo")
        self.assertAlmostEqual(r.evaluate(True), 8.8038, delta=0.001)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(True), 8.8038, delta=0.001)

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of AmbiguousRestraint via polymorphic pointer"""
        m, r = _make_test_restraint()
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        self.assertAlmostEqual(sf.evaluate(True), 8.8038, delta=0.001)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(True), 8.8038, delta=0.001)


if __name__ == '__main__':
    IMP.test.main()
