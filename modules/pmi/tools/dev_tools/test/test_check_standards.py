import unittest
import os
import sys
import re
import utils
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
check_py = os.path.join(TOPDIR, "check_standards.py")
sys.path.insert(0, TOPDIR)
try:
    import check_standards
finally:
    del sys.path[0]


class Tests(unittest.TestCase):

    def test_do_not_commit(self):
        """Test _check_do_not_commit function"""
        errors = []
        check_standards._check_do_not_commit("", 'foo', 0, errors)
        self.assertEqual(len(errors), 0)
        check_standards._check_do_not_commit("DO NOT COMMIT", 'foo', 0, errors)
        self.assertEqual(errors,
                         ['foo:1: Line contains the string "DO NOT COMMIT"'])

    def test_get_file(self):
        """Test get_file function"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, 'foo')
            utils.write_file(fname, 'foobar')
            fh, fn = check_standards.get_file(fname)
            self.assertEqual(fn, fname)
            self.assertEqual(fh.read(), 'foobar')

    def test_file_matches_re(self):
        """Test file_matches_re function"""
        excludes = [re.compile('foo')]
        self.assertFalse(check_standards.file_matches_re("bar", excludes))
        self.assertFalse(check_standards.file_matches_re("bar", []))
        self.assertTrue(check_standards.file_matches_re("foobar", excludes))

    def test_get_all_files(self):
        """Test get_all_files function"""
        with utils.RunInTempDir():
            for subdir in ('.sconf_temp', os.path.join('build', 'include'),
                           'ok'):
                os.makedirs(subdir)
                utils.write_file(os.path.join(subdir, "foo"), "")
            for f in ('.foo', 'foo', 'bar'):
                utils.write_file(f, "")
            fs = sorted(check_standards.get_all_files())
            self.assertEqual(fs, ['./bar', './foo', './ok/foo'])

    def test_complete(self):
        """Test simple complete run of check_standards script"""
        with utils.TempDir() as tmpdir:
            imp_info = os.path.join(tmpdir, ".imp_info.py")
            utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
            fname = os.path.join(tmpdir, "test.py")
            utils.write_file(fname, "# test file\n")
            p = subprocess.Popen([check_py], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)

    def test_check_python_file_bad_indentation(self):
        """Test check_python_file() with bad indentation"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, "test.py")
            utils.write_file(fname, "def foo():\n  pass\n")
            errors = []
            check_standards.check_python_file(fname, errors)
            self.assertEqual(len(errors), 1)
            self.assertTrue("please run through" in errors[0])

    def test_check_python_file_temp_test(self):
        """Test check_python_file() with temp test marker"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, "test.py")
            utils.write_file(fname, "class MyTest:\n"
                             "    def temp_hide_test_bar():\n        pass\n")
            errors = []
            check_standards.check_python_file(fname, errors)
            self.assertEqual(len(errors), 1)
            self.assertTrue(":2: Test case has the temp_hide_ prefix"
                            in errors[0])

    def test_check_python_file_incomp_merge(self):
        """Test check_python_file() with incomplete merge"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, "test.py")
            utils.write_file(fname, ">>>>>>> \n")
            errors = []
            check_standards.check_python_file(fname, errors)
            self.assertEqual(len(errors), 2)
            self.assertTrue("please run through" in errors[0])
            self.assertTrue(":1: error: Incomplete merge found."
                            in errors[1])

    def test_check_python_file_duplicate_tests(self):
        """Test check_python_file() with duplicate tests"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, "test.py")
            utils.write_file(fname, "class Tests:\n"
                                    "    def test_xyz():\n        pass\n"
                                    "    def test_xyz():\n        pass\n")
            errors = []
            check_standards.check_python_file(fname, errors)
            self.assertEqual(len(errors), 1)
            self.assertTrue(":4: Test case has multiple tests with "
                            "the same name test_xyz" in errors[0])

    def test_check_example_no_doxygen(self):
        """Test Python example with missing doxygen comments"""
        with utils.TempDir() as tmpdir:
            os.mkdir(os.path.join(tmpdir, 'examples'))
            fname = os.path.join(tmpdir, "examples", "my_example.py")
            utils.write_file(fname, "x = 42\n")
            errors = []
            check_standards.check_python_file(fname, errors)
            self.assertEqual(len(errors), 2)
            self.assertTrue(":1: Example does not have doxygen comments "
                            "at start" in errors[0])
            self.assertTrue(":1: Example \\example marker in first line"
                            in errors[1])

    def test_check_example_bad_import(self):
        """Test Python example with bad imports"""
        with utils.TempDir() as tmpdir:
            os.mkdir(os.path.join(tmpdir, 'examples'))
            fname = os.path.join(tmpdir, "examples", "my_example.py")
            utils.write_file(fname, """## \\example my_example.py
from foo import bar
import bar as baz
""")
            errors = []
            check_standards.check_python_file(fname, errors)
            self.assertEqual(len(errors), 2)
            self.assertTrue(":2: Examples should not use import from as "
                            "that confuses doxygen" in errors[0])
            self.assertTrue(":3: Examples should not rename types on import"
                            in errors[1])

    def test_check_c_file_incomp_merge(self):
        """Test check_c_file() with incomplete merge"""
        with utils.TempDir() as tmpdir:
            imp_info = os.path.join(tmpdir, ".imp_info.py")
            utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
            fname = os.path.join(tmpdir, "test.cpp")
            utils.write_file(fname, ">>>>>>> \n")
            errors = []
            check_standards.check_c_file(fname, errors)
            self.assertEqual(len(errors), 1)
            self.assertTrue(":1: error: Incomplete merge found."
                            in errors[0])

    def test_check_c_file_bad_define(self):
        """Test check_c_file() with bad #defines"""
        for ext in ('.cpp', '.h'):
            with utils.TempDir() as tmpdir:
                imp_info = os.path.join(tmpdir, ".imp_info.py")
                utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
                fname = os.path.join(tmpdir, "test" + ext)
                utils.write_file(fname,
                                 "#define FOO BAR\n#define IMP_FOO BAR\n"
                                 "#undef FOO\n"
                                 "#define BAZ BAR\n"
                                 "#define IMPTEST_FOO BAR\n"
                                 "#define EIGEN_YES_I_KNOW_SPARSE_"
                                 "MODULE_IS_NOT_STABLE_YET BAR\n")
                errors = []
                check_standards.check_c_file(fname, errors)
                if ext == '.h':
                    self.assertEqual(len(errors), 1)
                    self.assertTrue(":4: error: Preprocessor symbols must "
                                    "start with IMP_ or IMPTEST" in errors[0])
                else:
                    self.assertEqual(len(errors), 0)

    def test_check_c_file_leading_blanks(self):
        """Test check_c_file() with leading blank lines"""
        with utils.TempDir() as tmpdir:
            imp_info = os.path.join(tmpdir, ".imp_info.py")
            utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
            fname = os.path.join(tmpdir, "test.cpp")
            utils.write_file(fname, "\nint x;\n")
            errors = []
            check_standards.check_c_file(fname, errors)
            self.assertEqual(len(errors), 1)
            self.assertTrue(":1: File has leading blank line(s)" in errors[0])

    def test_check_c_file_missing_file(self):
        """Test check_c_file() with missing \\file marker"""
        for subdir in ('include', 'internal'):
            with utils.TempDir() as tmpdir:
                imp_info = os.path.join(tmpdir, ".imp_info.py")
                utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
                os.mkdir(os.path.join(tmpdir, subdir))
                fname = os.path.join(tmpdir, subdir, "test.h")
                utils.write_file(fname, "int x;\n")
                errors = []
                check_standards.check_c_file(fname, errors)
                if subdir == 'include':
                    self.assertEqual(len(errors), 1)
                    self.assertTrue(":2: Exported header must have a "
                                    "line \\file IMP/test/test.h" in errors[0])
                else:
                    self.assertEqual(len(errors), 0)

    def test_check_c_file_cpp_ok(self):
        """Test check_c_file() with ok cpp file"""
        with utils.TempDir() as tmpdir:
            imp_info = os.path.join(tmpdir, ".imp_info.py")
            utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
            fname = os.path.join(tmpdir, "test.cpp")
            utils.write_file(fname, "int x;\n")
            errors = []
            check_standards.check_c_file(fname, errors)
            self.assertEqual(len(errors), 0)

    def test_check_c_file_header_ok(self):
        """Test check_c_file() with ok header file"""
        with utils.TempDir() as tmpdir:
            os.makedirs(os.path.join(tmpdir, 'IMP', 'test', 'include'))
            imp_info = os.path.join(tmpdir, ".imp_info.py")
            utils.write_file(imp_info, '{\n  "name": "IMP.test"\n}\n')
            fname = os.path.join(tmpdir, "IMP", "test", "include", "foo.h")
            utils.write_file(fname, "\\file IMP/test/include/foo.h'")
            errors = []
            check_standards.check_c_file(fname, errors)
            self.assertEqual(len(errors), 0)


if __name__ == '__main__':
    unittest.main()
