from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example

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

if __name__ == '__main__':
    IMP.test.main()
