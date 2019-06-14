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

    def test_missing_input_file(self):
        """Test create_gmm given missing input file"""
        args = MockArgs()
        args.data_file = '/not/exist/foo'
        args.n_centers = 100
        args.out_file = 'outfile'
        self.assertRaises(Exception, create_gmm.run, args)

    def test_badext_input_file(self):
        """Test create_gmm given input file with bad extension"""
        self.assertRaises(ValueError, self.run_python_module, create_gmm,
                [self.get_input_file_name('sequence.dat'), '4',
                 'create_gmm_pdb.txt'])

    def test_from_pdb(self):
        """Test create_gmm from a PDB file"""
        try:
            import sklearn.mixture
        except ImportError:
            self.skipTest("this test requires the sklearn Python module")
        self.run_python_module(create_gmm,
                [self.get_input_file_name('twoblobsA.pdb'), '4',
                 'create_gmm_pdb.txt'])
        with open('create_gmm_pdb.txt') as fh:
            lines = fh.readlines()
        self.assertEqual(len(lines), 19)
        self.assertEqual(lines[0][:44],
                '# Created by create_gmm.py, IMP.isd version ')
        self.assertEqual(lines[2], "# ncenters: 4\n")
        self.assertEqual(lines[13], "# chain: None\n")
        os.unlink('create_gmm_pdb.txt')

    def test_from_pdb_chain(self):
        """Test create_gmm from a single chain of a PDB file"""
        try:
            import sklearn.mixture
        except ImportError:
            self.skipTest("this test requires the sklearn Python module")
        self.run_python_module(create_gmm,
                [self.get_input_file_name('twoblobsA.pdb'), '4',
                 'create_gmm_pdb_chain.txt',
                 '--chain', 'A', '--multiply_by_mass'])
        with open('create_gmm_pdb_chain.txt') as fh:
            lines = fh.readlines()
        self.assertEqual(len(lines), 19)
        self.assertEqual(lines[0][:44],
                '# Created by create_gmm.py, IMP.isd version ')
        self.assertEqual(lines[2], "# ncenters: 4\n")
        self.assertEqual(lines[13], "# chain: 'A'\n")
        os.unlink('create_gmm_pdb_chain.txt')

    def test_from_mrc(self):
        """Test create_gmm from an MRC file"""
        try:
            import sklearn.mixture
        except ImportError:
            self.skipTest("this test requires the sklearn Python module")
        self.run_python_module(create_gmm,
                ['--out_map', 'create_gmm_mrc.mrc',
                 self.get_input_file_name('twoblobs-4.0.mrc'), '4',
                 'create_gmm_mrc.txt', '--force_weight_frac',
                 '--use_dirichlet'])
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
