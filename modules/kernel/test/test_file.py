from __future__ import print_function
import IMP.test
import os.path
import sys
from io import BytesIO, StringIO


class Tests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test that conversions to input files work properly"""
        print("starting")
        IMP.set_log_level(IMP.MEMORY)
        v = IMP._test_ifile(self.get_input_file_name("text"))
        self.assertEqual(v, "word")
        with self.open_input_file("text") as fh:
            v = IMP._test_ifile(fh)
        self.assertEqual(v, "word")
        self.assertRaises(IOError, IMP._test_ifile, "notafile")
        s = BytesIO(b"hi there")
        v = IMP._test_ifile(s)
        self.assertEqual(v, "hithere")
        print("done")

    def test_odata_directory(self):
        """Test that conversions to output files work properly"""
        print("starting")
        IMP.set_log_level(IMP.MEMORY)
        IMP._test_ofile("ofile_test")
        self.assertRaises(IOError, IMP._test_ofile, "nodir/hi")
        with open("hi", "w") as f:
            IMP._test_ofile(f)
        # In Python 3 binary files are handled differently (as raw bytes,
        # not Unicode)
        with open("hi", "wb") as f:
            IMP._test_ofile(f)
        s = BytesIO()
        IMP._test_ofile(s)
        self.assertTrue(s.getvalue().startswith(b"hi\n"))
        if sys.version_info[0] >= 3:
            s = StringIO()
            IMP._test_ofile(s)
            self.assertTrue(s.getvalue().startswith("hi\n"))
        self.assertRaises(TypeError, IMP._test_ofile, 1)

        class NoMethods(object):
            pass
        self.assertRaises(TypeError, IMP._test_ofile, NoMethods)
        del f
        print("unlinking")
        # os.unlink('ofile_test')
        # os.unlink('hi')
        print("done")

    def test_odata_directory_2(self):
        """Test temporary files"""
        simple = IMP.create_temporary_file()
        print(simple.get_name())
        prefix = IMP.create_temporary_file("imp")
        print(prefix.get_name())
        self.assertIn("imp", prefix.get_name())
        suffix = IMP.create_temporary_file("imp", ".py")
        print(suffix.get_name())
        self.assertIn("imp", suffix.get_name())
        self.assertIn(".py", suffix.get_name())

    def test_bad(self):
        """Test bad paths trigger IO exceptions with overloads"""
        self.assertRaises(
            IOError,
            IMP._test_ifile_overloaded,
            "bad path",
            "hi")
        s = BytesIO()
        # shouldn't raise
        IMP._test_ifile_overloaded(s, "hi")

    def test_abspath(self):
        """Test get_absolute_path()"""
        relpth = "bar/foo.pdb"
        abspth = os.path.abspath(relpth)

        # Absolute path should be returned unchanged
        self.assertEqual(IMP.get_absolute_path(abspth), abspth)

        # Relative path should be converted to abspath, except on Windows
        if sys.platform == 'win32':
            self.assertEqual(IMP.get_absolute_path(relpth), relpth)
        else:
            self.assertEqual(IMP.get_absolute_path(relpth), abspth)

if __name__ == '__main__':
    IMP.test.main()
