import IMP
import IMP.test
import IMP.core
try:
    import jax
except ImportError:
    jax = None


fkey = IMP.FloatKey("my float key")


def make_score(key):
    m = IMP.Model()
    p1 = IMP.Particle(m)
    IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(5., 6., 7.))
    p1.add_attribute(fkey, 42.0)
    s = IMP.core.AttributeSingletonScore(IMP.core.Linear(0.0, 10.0), key)
    r = IMP.core.SingletonRestraint(m, s, p1)
    return m, p1, s, r


class Tests(IMP.test.TestCase):

    def test_score(self):
        """Test AttributeSingletonScore value"""
        m, p1, s, r = make_score(IMP.core.XYZ.get_xyz_keys()[0])
        self.assertAlmostEqual(r.evaluate(False), 50.0, delta=1e-4)

        m, p1, s, r = make_score(fkey)
        self.assertAlmostEqual(r.evaluate(False), 420.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
