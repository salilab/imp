from __future__ import print_function
import IMP
import IMP.test


class Tests(IMP.test.TestCase):

    def test_int(self):
        """Test get/set of int RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_int(), 0)
        ri.add_int("test int", 42)
        self.assertEqual(ri.get_number_of_int(), 1)
        self.assertEqual(ri.get_int_key(0), "test int")
        self.assertEqual(ri.get_int_value(0), 42)

    def test_float(self):
        """Test get/set of float RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_float(), 0)
        ri.add_float("test float", 4.5)
        self.assertEqual(ri.get_number_of_float(), 1)
        self.assertEqual(ri.get_float_key(0), "test float")
        self.assertAlmostEqual(ri.get_float_value(0), 4.5, delta=1e-6)

    def test_string(self):
        """Test get/set of string RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_string(), 0)
        ri.add_string("test string", "foo")
        self.assertEqual(ri.get_number_of_string(), 1)
        self.assertEqual(ri.get_number_of_filename(), 0)
        self.assertEqual(ri.get_string_key(0), "test string")
        self.assertEqual(ri.get_string_value(0), "foo")

    def test_filename(self):
        """Test get/set of filename RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_filename(), 0)
        ri.add_filename("test filename", "foo")
        self.assertEqual(ri.get_number_of_filename(), 1)
        self.assertEqual(ri.get_number_of_string(), 0)
        self.assertEqual(ri.get_filename_key(0), "test filename")
        self.assertEqual(ri.get_filename_value(0), "foo")

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

    def test_filenames(self):
        """Test get/set of filenames RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_filenames(), 0)
        ri.add_filenames("test filenames", ["/foo/bar", "/foo/baz"])
        self.assertEqual(ri.get_number_of_filenames(), 1)
        self.assertEqual(ri.get_filenames_key(0), "test filenames")
        self.assertEqual(ri.get_filenames_value(0), ["/foo/bar", "/foo/baz"])

if __name__ == '__main__':
    IMP.test.main()
