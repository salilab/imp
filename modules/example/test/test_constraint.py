from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example

class Tests(IMP.test.TestCase):

    def test_constraint(self):
        """Test example Constraint"""
        k = IMP.IntKey("Constraint key")
        # Test both implementations: C++ and Python
        for typ in (IMP.example.ExampleConstraint,
                    IMP.example.PythonExampleConstraint):
            m = IMP.Model()
            p = IMP.Particle(m)
            c = typ(p)
            self.assertEqual(p.get_value(k), 0)
            m.update()
            self.assertEqual(p.get_value(k), 1)
            m.update()
            self.assertEqual(p.get_value(k), 2)
            self.assertIn("ExampleConstraint", str(c))
            self.assertIn("ExampleConstraint", repr(c))
            self.assertIn("example", c.get_version_info().get_module())
            self.assertEqual(len(c.get_inputs()), 1)
            self.assertEqual(len(c.get_outputs()), 1)

if __name__ == '__main__':
    IMP.test.main()
