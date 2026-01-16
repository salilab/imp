import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example
import pickle
try:
    import jax
except ImportError:
    jax = None


def make_restraint():
    m = IMP.Model()
    p = m.add_particle("p")
    d = IMP.core.XYZ.setup_particle(m, p, IMP.algebra.Vector3D(1,2,3))
    r = IMP.example.ExampleRestraint(m, p, 10.)
    return m, r


class Tests(IMP.test.TestCase):

    def test_restraint(self):
        """Test example restraint"""
        m = IMP.Model()
        p = m.add_particle("p")
        d = IMP.core.XYZ.setup_particle(m, p, IMP.algebra.Vector3D(1,2,3))
        # Test both implementations: C++ and Python
        for typ in (IMP.example.ExampleRestraint,
                    IMP.example.PythonExampleRestraint):
            r = typ(m, p, 10.)
            self.assertAlmostEqual(r.evaluate(True), 45.0, delta=1e-4)
            self.assertLess(IMP.algebra.get_distance(d.get_derivatives(),
                                                IMP.algebra.Vector3D(0,0,30)),
                            1e-4)
            self.assertIn("ExampleRestraint", str(r))
            self.assertIn("ExampleRestraint", repr(r))
            self.assertIn("example", r.get_version_info().get_module())
            self.assertEqual(len(r.get_inputs()), 1)

    def test_serialize(self):
        """Test (un-)serialize of ExampleRestraint"""
        m, r = make_restraint()
        self.assertAlmostEqual(r.evaluate(False), 45.0, delta=1e-3)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 45.0, delta=1e-3)

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of ExampleRestraint via polymorphic pointer"""
        m, r = make_restraint()
        sf = IMP.core.RestraintsScoringFunction([r])
        self.assertAlmostEqual(sf.evaluate(False), 45.0, delta=1e-3)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        self.assertAlmostEqual(newsf.evaluate(False), 45.0, delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation"""
        # Test both classes: C++ and Python
        for typ in (IMP.example.ExampleRestraint,
                    IMP.example.PythonExampleRestraint):
            m = IMP.Model()
            p = m.add_particle("p")
            d = IMP.core.XYZ.setup_particle(m, p, IMP.algebra.Vector3D(1,2,3))
            r = typ(m, p, 10.)
            ji = r._get_jax()
            X = ji.get_model_state()
            s = jax.jit(ji.score_func)
            self.assertAlmostEqual(s(X), 45.0, delta=1e-3)
            g = jax.jit(jax.grad(ji.score_func))
            self.assertLess(IMP.algebra.get_distance(
                g(X)['xyz'][0], IMP.algebra.Vector3D(0,0,30)), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
