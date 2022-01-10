import unittest
import subprocess
import os
import utils

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
header_py = os.path.join(TOPDIR, 'make_all_header.py')


class Tests(unittest.TestCase):

    def test_header(self):
        """Test make_all_header.py script"""
        with utils.TempDir() as tmpdir:
            topdir = os.path.join(tmpdir, 'include', 'test')
            os.makedirs(topdir)
            os.mkdir(os.path.join(topdir, 'subdir'))
            explicit_h = os.path.join(topdir, 'explicit.h')
            sub_h = os.path.join(topdir, 'subdir', 'sub.h')
            sub_deprec_h = os.path.join(topdir, 'subdir', 'deprecated.h')
            utils.write_file(explicit_h, '')
            utils.write_file(sub_h, "sub_h")
            utils.write_file(sub_deprec_h, "DEPRECATED_HEADER")
            p = subprocess.Popen([header_py, 'include/test.h', 'bar',
                                  'include/test/explicit.h',
                                  'include/test/subdir'], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)
            self.assertEqual(utils.read_file(os.path.join(tmpdir,
                                                          'include/test.h')),
                             """/**
 *  \\file test.h
 *  \\brief Include all non-deprecated headers in test.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef TEST_H
#define TEST_H
#include <bar/explicit.h>
#include <bar/sub.h>
#ifdef IMP_SWIG_WRAPPER
#include <bar/deprecated.h>
#endif
#endif /* TEST_H */
""")


if __name__ == '__main__':
    unittest.main()
