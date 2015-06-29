from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example

class Tests(IMP.test.TestCase):

    def test_unary_function(self):
        """Test example UnaryFunction"""
        # Test both implementations: C++ and Python
        for typ in (IMP.example.ExampleUnaryFunction,
                    IMP.example.PythonExampleUnaryFunction):
            u = typ(2.0, 10.0)
            self.assertAlmostEqual(u.evaluate(5.0), 45.0, delta=1e-4)
            dp = u.evaluate_with_derivative(5.0)
            self.assertAlmostEqual(dp[0], 45.0, delta=1e-4)
            self.assertAlmostEqual(dp[1], 30.0, delta=1e-4)
            self.assertIn("UnaryFunction", str(u))
            self.assertIn("UnaryFunction", repr(u))
            self.assertIn("example", u.get_version_info().get_module())

    def test_combine(self):
        """Test combining example UnaryFunction with a PairScore"""
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        for typ in (IMP.example.ExampleUnaryFunction,
                    IMP.example.PythonExampleUnaryFunction):
            u = typ(2.0, 10.0)
            ps = IMP.core.DistancePairScore(u)
            da = IMP.DerivativeAccumulator()
            self.assertAlmostEqual(ps.evaluate_index(m, [p1, p2], da),
                                   51.08, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
