import unittest
import os
import sys
import re
import utils
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, TOPDIR)
try:
    from python_tools import cpp_format
finally:
    del sys.path[0]

from pygments import token

# Tokens we'd expect to see in an OK header
ok_header_tokens = [
  (token.Comment.Multiline, "/* Comment"),
  (token.Text, "\n"),
  (token.Text, "\n"),
  (token.Comment.Preproc, "#"),
  (token.Comment.Preproc, "ifndef IMPKERNEL_TEST_H"),
  (token.Comment.Preproc, "\n"),
  (token.Comment.Preproc, "#"),
  (token.Comment.Preproc, "define IMPKERNEL_TEST_H"),
  (token.Comment.Preproc, "\n"),
  (token.Text, "\n"),
  (token.Comment.Preproc, '#'),
  (token.Comment.Preproc, 'endif '),
  (token.Comment.Multiline, '/* IMPKERNEL_TEST_H */'),
  (token.Comment.Preproc, '\n')
]

class Tests(unittest.TestCase):

    def test_have_header_guard(self):
        """Test have_header_guard()"""
        self.assertTrue(cpp_format.have_header_guard(ok_header_tokens))
        bad_tokens = ok_header_tokens[:]
        del bad_tokens[1]
        self.assertFalse(cpp_format.have_header_guard(bad_tokens))

    def test_get_header_guard(self):
        """Test get_header_guard()"""
        self.assertEqual(cpp_format.get_header_guard("/foo/bar/FooHeader.h",
                                                     "IMP.mytest"),
                         ("IMPMYTEST", "FOOHEADER_H"))

    def check_header_start_end(self, tokens):
        errors = []
        cpp_format.check_header_start_end(tokens, "/foo/bar/Test.h",
                                          "IMP.kernel", errors)
        return errors

    def assertMissingGuard(self, error):
        self.assertTrue(":1: Missing or incomplete header guard." in error)

    def test_check_header_start_end_ok(self):
        """Test check_header_start_end with an OK header"""
        errors = self.check_header_start_end(ok_header_tokens)
        self.assertEqual(len(errors), 0)

    def test_check_header_start_end_too_short(self):
        """Test check_header_start_end with too few tokens"""
        errors = self.check_header_start_end([])
        self.assertEqual(len(errors), 1)
        self.assertMissingGuard(errors[0])

    def test_check_header_start_end_missing_ifndef(self):
        """Test check_header_start_end with missing ifndef"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[4] = (token.Comment.Preproc, "garbage IMPKERNEL_TEST_H")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 5)
        self.assertTrue(":1: Header guard missing #ifndef." in errors[0])
        self.assertMissingGuard(errors[4])

    def test_check_header_start_end_missing_ifndef_preproc(self):
        """Test check_header_start_end with missing ifndef preproc"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[4] = (token.Text, "ifndef IMPKERNEL_TEST_H")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 1)
        self.assertMissingGuard(errors[0])

    def test_check_header_start_end_missing_define(self):
        """Test check_header_start_end with missing define"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[7] = (token.Comment.Preproc, "garbage IMPKERNEL_TEST_H")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 3)
        self.assertTrue(":1: Header guard missing #define." in errors[0])
        self.assertTrue('1: Header guard does not define "IMPKERNEL_TEST_H"'
                        in errors[1])
        self.assertMissingGuard(errors[2])

    def test_check_header_start_end_missing_define_preproc(self):
        """Test check_header_start_end with missing define preproc"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[7] = (token.Text, "define IMPKERNEL_TEST_H")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 1)
        self.assertMissingGuard(errors[0])

    def test_check_header_start_end_missing_endif(self):
        """Test check_header_start_end with missing endif"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[-3] = (token.Comment.Preproc, "garbage")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 2)
        self.assertTrue(":1: Header guard missing #endif." in errors[0])
        self.assertMissingGuard(errors[1])

    def test_check_header_start_end_missing_endif_preproc(self):
        """Test check_header_start_end with missing endif preproc"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[-4] = (token.Text, "#")
        bad_tokens[-3] = (token.Text, "endif")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 1)
        self.assertMissingGuard(errors[0])

    def test_check_header_start_end_missing_close_comment(self):
        """Test check_header_start_end with missing closing comment"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[-2] = (token.Text, '/* IMPKERNEL_TEST_H */')
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 2)
        self.assertTrue(":1: Header guard missing closing comment."
                        in errors[0])
        self.assertMissingGuard(errors[1])

    def test_check_header_start_end_wrong_close_comment(self):
        """Test check_header_start_end with wrong closing comment"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[-2] = (token.Comment.Multiline, '/* IMPKERNEL_FOO_H */')
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 2)
        self.assertTrue(':1: Header guard close does not have a comment '
                        'of "/* IMPKERNEL_TEST_H */".' in errors[0])
        self.assertMissingGuard(errors[1])

    def test_check_header_start_end_bad_guard_prefix(self):
        """Test check_header_start_end with bad header guard prefix"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[4] = (token.Comment.Preproc, "ifndef bad_FOO_H")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 5)
        self.assertTrue(':1: Header guard does not start with "IMPKERNEL".'
                        in errors[0])
        self.assertMissingGuard(errors[4])

    def test_check_header_start_end_bad_guard_suffix(self):
        """Test check_header_start_end with bad header guard suffix"""
        bad_tokens = ok_header_tokens[:]
        bad_tokens[4] = (token.Comment.Preproc, "ifndef IMPKERNEL_bad")
        errors = self.check_header_start_end(bad_tokens)
        self.assertEqual(len(errors), 4)
        self.assertTrue(':1: Header guard does not end with "TEST_H".'
                        in errors[0])
        self.assertMissingGuard(errors[3])

    def test_check_comment_header_ok(self):
        """Test check_comment_header with OK comment"""
        errors = []
        cpp_format.check_comment_header(ok_header_tokens, "/foo/bar/Test.h",
                                        errors)
        self.assertEqual(errors, [])

    def test_check_comment_header_empty(self):
        """Test check_comment_header with empty file"""
        errors = []
        cpp_format.check_comment_header([], "/foo/bar/Test.h", errors)
        self.assertEqual(errors, ['/foo/bar/Test.h:1: First line should be '
                                  'a comment with a copyright notice and a '
                                  'description of the file'])

    def test_check_comment_header_not_comment(self):
        """Test check_comment_header with something not a comment"""
        errors = []
        cpp_format.check_comment_header([(token.Text, "\n")],
                                        "/foo/bar/Test.h", errors)
        self.assertEqual(errors, ['/foo/bar/Test.h:1: First line should be '
                                  'a comment with a copyright notice and a '
                                  'description of the file'])

    def test_tokenize_file(self):
        """Test tokenize_file()"""
        class DummyFile(object):
            def read(self):
                return None
        fh = DummyFile()
        self.assertEqual(cpp_format.tokenize_file(fh), [("tok1", "val1"),
                                                        ("tok2", "val2")])

    def test_check_header_file(self):
        """Test check_header_file()"""
        def ok_tokenize(fh):
            return ok_header_tokens
        with utils.mocked_function(cpp_format, 'tokenize_file', ok_tokenize):
            errors = []
            cpp_format.check_header_file((None, "/foo/bar/test.h"),
                                         "IMP.kernel", errors)
            self.assertEqual(errors, [])

    def test_check_cpp_file(self):
        """Test check_cpp_file()"""
        def ok_tokenize(fh):
            return ok_header_tokens
        with utils.mocked_function(cpp_format, 'tokenize_file', ok_tokenize):
            errors = []
            cpp_format.check_cpp_file((None, "/foo/bar/test.cpp"),
                                      "IMP.kernel", errors)
            self.assertEqual(errors, [])

    def test_check_cpp_file_test(self):
        """Test check_cpp_file() with a test file"""
        def empty_tokenize(fh):
            return []
        with utils.mocked_function(cpp_format, 'tokenize_file', empty_tokenize):
            errors = []
            cpp_format.check_cpp_file((None, "/foo/bar/test_foo.cpp"),
                                      "IMP.kernel", errors)
            self.assertEqual(errors, [])

if __name__ == '__main__':
    unittest.main()
