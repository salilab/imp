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
        ri.clear()
        self.assertEqual(ri.get_number_of_int(), 0)

    def test_float(self):
        """Test get/set of float RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_float(), 0)
        ri.add_float("test float", 4.5)
        self.assertEqual(ri.get_number_of_float(), 1)
        self.assertEqual(ri.get_float_key(0), "test float")
        self.assertAlmostEqual(ri.get_float_value(0), 4.5, delta=1e-6)
        ri.clear()
        self.assertEqual(ri.get_number_of_float(), 0)

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
        ri.clear()
        self.assertEqual(ri.get_number_of_string(), 0)

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
        ri.clear()
        self.assertEqual(ri.get_number_of_filename(), 0)

    def test_filename_string_naming(self):
        """Test checks of suitable names for string or filename info"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertRaisesUsageException(ri.add_string, "test filename", "foo")
        self.assertRaisesUsageException(ri.add_strings, "test filename",
                                        ["foo", "bar"])
        self.assertRaisesUsageException(ri.add_filename, "test string", "foo")
        self.assertRaisesUsageException(ri.add_filenames, "test string",
                                        ["foo", "bar"])

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
        ri.clear()
        self.assertEqual(ri.get_number_of_floats(), 0)

    def test_ints(self):
        """Test get/set of ints RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_ints(), 0)
        ri.add_ints("test ints", [42, 100])
        self.assertEqual(ri.get_number_of_ints(), 1)
        self.assertEqual(ri.get_ints_key(0), "test ints")
        v = ri.get_ints_value(0)
        self.assertEqual(list(v), [42, 100])
        ri.clear()
        self.assertEqual(ri.get_number_of_ints(), 0)

    def test_strings(self):
        """Test get/set of strings RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_strings(), 0)
        ri.add_strings("test strings", ["foo", "bar"])
        self.assertEqual(ri.get_number_of_strings(), 1)
        self.assertEqual(ri.get_strings_key(0), "test strings")
        self.assertEqual(ri.get_strings_value(0), ["foo", "bar"])
        ri.clear()
        self.assertEqual(ri.get_number_of_strings(), 0)

    def test_filenames(self):
        """Test get/set of filenames RestraintInfo"""
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_filenames(), 0)
        ri.add_filenames("test filenames", ["/foo/bar", "/foo/baz"])
        self.assertEqual(ri.get_number_of_filenames(), 1)
        self.assertEqual(ri.get_filenames_key(0), "test filenames")
        self.assertEqual(ri.get_filenames_value(0), ["/foo/bar", "/foo/baz"])
        ri.clear()
        self.assertEqual(ri.get_number_of_filenames(), 0)

    def test_particle_indexes(self):
        """Test get/set of ParticleIndexes RestraintInfo"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        ri = IMP.RestraintInfo()
        ri.set_was_used(True)
        self.assertEqual(ri.get_number_of_particle_indexes(), 0)
        ri.add_particle_indexes("test pis", [p1, p2])
        self.assertEqual(ri.get_number_of_particle_indexes(), 1)
        self.assertEqual(ri.get_particle_indexes_key(0), "test pis")
        v = ri.get_particle_indexes_value(0)
        self.assertNumPyArrayEqual(
            v, [p1.get_index(), p2.get_index()])
        ri.clear()
        self.assertEqual(ri.get_number_of_particle_indexes(), 0)


if __name__ == '__main__':
    IMP.test.main()
