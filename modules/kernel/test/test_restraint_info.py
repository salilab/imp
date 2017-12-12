from __future__ import print_function
import IMP
import IMP.test


class Tests(IMP.test.TestCase):

    def test_floats(self):
        """Test get/set of floats RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_floats(), 0)
        ri.add_floats("test floats", [42., 100.])
        self.assertEqual(ri.get_number_of_floats(), 1)
        self.assertEqual(ri.get_floats_key(0), "test floats")
        v = ri.get_floats_value(0)
        self.assertEqual(len(v), 2)
        self.assertAlmostEqual(v[0], 42., delta=1e-6)
        self.assertAlmostEqual(v[1], 100., delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
