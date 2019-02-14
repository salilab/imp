#!/usr/bin/env python

import IMP
import IMP.core
from IMP.isd import create_gmm
import IMP.test

class MockArgs(object):
    pass

class Tests(IMP.test.TestCase):

    def test_create_gmm_help(self):
        """Test create_gmm help"""
        self.check_runnable_python_module("IMP.isd.create_gmm")

    def test_create_gmm_usage(self):
        """Test create_gmm module incorrect usage"""
        r = self.run_python_module("IMP.isd.create_gmm", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertRegex(err, "(too few arguments|arguments are required)")
        self.assertNotEqual(r.returncode, 0)

    def test_bad_input_file(self):
        """Test create_gmm given bad input file"""
        args = MockArgs()
        args.data_file = '/not/exist/foo'
        args.n_centers = 100
        args.out_file = 'outfile'
        self.assertRaises(Exception, create_gmm.run, args)


if __name__ == '__main__':
    IMP.test.main()
