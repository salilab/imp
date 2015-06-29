from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example

class Tests(IMP.test.TestCase):

    def test_pair_score(self):
        """Test example PairScore"""
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        # Test both implementations: C++ and Python
        for typ in (IMP.example.ExamplePairScore,
                    IMP.example.PythonExamplePairScore):
            p = typ(2.0, 10.0)
            da = IMP.DerivativeAccumulator()
            self.assertAlmostEqual(p.evaluate_index(m, [p1, p2], da),
                                   51.08, delta=0.01)
            # Note that we can't test derivatives because they haven't been
            # initialized
            self.assertIn("PairScore", str(p))
            self.assertIn("PairScore", repr(p))
            self.assertIn("example", p.get_version_info().get_module())
            self.assertEqual(len(p.get_inputs(m, [p1,p2])), 2)

if __name__ == '__main__':
    IMP.test.main()
