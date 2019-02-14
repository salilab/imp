#!/usr/bin/env python

import IMP
import IMP.core
from IMP.isd import create_gmm
import IMP.test
import sys
import os

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

    def test_from_pdb(self):
        """Test create_gmm from a PDB file"""
        try:
            import sklearn.mixture
        except ImportError:
            self.skipTest("this test requires the sklearn Python module")
        r = self.run_python_module("IMP.isd.create_gmm",
                [self.get_input_file_name('twoblobsA.pdb'), '4',
                 'create_gmm_pdb.txt'])
        out, err = r.communicate()
        sys.stdout.write(out)
        sys.stderr.write(err)
        self.assertEqual(r.returncode, 0)
        with open('create_gmm_pdb.txt') as fh:
            lines = fh.readlines()
        self.assertEqual(len(lines), 19)
        self.assertEqual(lines[0][:44],
                '# Created by create_gmm.py, IMP.isd version ')
        self.assertEqual(lines[2], "# ncenters: 4\n")
        os.unlink('create_gmm_pdb.txt')

    def test_from_mrc(self):
        """Test create_gmm from an MRC file"""
        try:
            import sklearn.mixture
        except ImportError:
            self.skipTest("this test requires the sklearn Python module")
        r = self.run_python_module("IMP.isd.create_gmm",
                ['--out_map', 'create_gmm_mrc.mrc',
                 self.get_input_file_name('twoblobs-4.0.mrc'), '4',
                 'create_gmm_mrc.txt'])
        out, err = r.communicate()
        sys.stdout.write(out)
        sys.stderr.write(err)
        self.assertEqual(r.returncode, 0)
        with open('create_gmm_mrc.txt') as fh:
            lines = fh.readlines()
        self.assertEqual(len(lines), 19)
        self.assertEqual(lines[0][:44],
                '# Created by create_gmm.py, IMP.isd version ')
        self.assertEqual(lines[2], "# ncenters: 4\n")
        os.unlink('create_gmm_mrc.txt')
        os.unlink('create_gmm_mrc.mrc')


if __name__ == '__main__':
    IMP.test.main()
