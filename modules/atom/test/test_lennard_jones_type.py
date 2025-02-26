import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra


def _make_test_decorator(m):
    p = IMP.Particle(m)
    v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
    s = IMP.algebra.Sphere3D(v, 4.0)
    IMP.core.XYZR.setup_particle(p, s)
    return IMP.atom.LennardJones.setup_particle(p, 0.5)


class Tests(IMP.test.TestCase):

    """Test the LennardJonesTyped decorator and LennardJonesType"""

    def test_make_type(self):
        """Check creation of LennardJonesType"""
        t = IMP.atom.LennardJonesType(2.0, 1.0, "type x")
        self.assertAlmostEqual(t.get_well_depth(), 2.0, delta=1e-6)
        self.assertAlmostEqual(t.get_radius(), 1.0, delta=1e-6)
        self.assertEqual(t.get_name(), "type x")
        t.set_well_depth(3.0)
        t.set_radius(2.0)
        self.assertAlmostEqual(t.get_well_depth(), 3.0, delta=1e-6)
        self.assertAlmostEqual(t.get_radius(), 2.0, delta=1e-6)

    def test_create(self):
        """Check creation of LennardJonesTyped decorators"""
        t = IMP.atom.LennardJonesType(2.0, 1.0, "type x")
        m = IMP.Model()
        p = IMP.Particle(m)
        coord = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p, coord)
        typ = IMP.atom.LennardJonesTyped.setup_particle(p, t)
        self.assertAlmostEqual(typ.get_well_depth(), 2.0, delta=1e-6)
        self.assertAlmostEqual(typ.get_radius(), 1.0, delta=1e-6)
        t2 = typ.get_type()
        self.assertAlmostEqual(t2.get_radius(), t.get_radius())
        self.assertAlmostEqual(t2.get_well_depth(), t.get_well_depth())
        self.assertEqual(t2.get_name(), t.get_name())
        # Change to LennardJonesType should be reflected in the decorator
        t.set_well_depth(3.0)
        t.set_radius(4.0)
        self.assertAlmostEqual(typ.get_well_depth(), 3.0, delta=1e-6)
        self.assertAlmostEqual(typ.get_radius(), 4.0, delta=1e-6)

        # Particle must already be an XYZ
        p = IMP.Particle(m)
        self.assertRaisesUsageException(
            IMP.atom.LennardJonesTyped.setup_particle, p, t)


if __name__ == '__main__':
    IMP.test.main()
