from __future__ import print_function
import IMP.test
import sys
import os
import re
import shutil


class SAXSToolsTest(IMP.test.ApplicationTestCase):

    def test_compute_rg(self):
        """Simple test of Rg calculation"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('compute_rg',
                                 [self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('lyzexp.dat Rg= ([\d\.]+)', out)
        self.assertIsNotNone(m, msg="Rg value output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 15.14, delta=0.1)
        m = re.search('6lyz.pdb Rg= ([\d\.]+)', out)
        self.assertIsNotNone(m, msg="Rg value output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 13.966, delta=0.1)

    def test_compute_rg_cif(self):
        """Simple test of Rg calculation with mmCIF input"""
        p = self.run_application('compute_rg',
                                 [self.get_input_file_name('6lyz.cif'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('lyzexp.dat Rg= ([\d\.]+)', out)
        self.assertIsNotNone(m, msg="Rg value output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 15.14, delta=0.1)
        m = re.search('6lyz.cif Rg= ([\d\.]+)', out)
        self.assertIsNotNone(m, msg="Rg value output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 13.966, delta=0.1)

    def test_compute_chi(self):
        """Simple test of Chi calculation"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('compute_chi',
                                 [self.get_input_file_name('lyzexp.dat'),
                                  self.get_input_file_name('6lyz.pdb.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('6lyz.pdb.dat chi=([\d\.]+)', out)
        self.assertIsNotNone(
            m,
            msg="chi value output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.288, delta=0.1)

    def test_compute_vr(self):
        """Simple test of VR calculation"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('compute_vr',
                                 [self.get_input_file_name('lyzexp.dat'),
                                  self.get_input_file_name('6lyz.pdb.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('6lyz.pdb.dat vr=([\d\.]+)', out)
        self.assertIsNotNone(
            m,
            msg="vr value output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 5.78, delta=0.1)

    def test_validate_profile(self):
        """Simple test of validate_profile tool"""
        with IMP.test.temporary_directory() as tmpdir:
            shutil.copy(self.get_input_file_name('weighted.dat'), tmpdir)
            p = self.run_application('validate_profile', ['weighted.dat'],
                                     cwd=tmpdir)
            out, err = p.communicate()
            sys.stderr.write(err)
            self.assertApplicationExitedCleanly(p.returncode, err)
            with open(os.path.join(tmpdir, 'weighted_v.dat')) as fh:
                wc = len(fh.readlines())
            self.assertEqual(wc, 503)

    def test_validate_profile_max_q(self):
        """Simple test of validate_profile tool with maxq set"""
        with IMP.test.temporary_directory() as tmpdir:
            shutil.copy(self.get_input_file_name('weighted.dat'), tmpdir)
            p = self.run_application('validate_profile',
                                     ['-q', '0.4', 'weighted.dat'],
                                     cwd=tmpdir)
            out, err = p.communicate()
            sys.stderr.write(err)
            self.assertApplicationExitedCleanly(p.returncode, err)
            with open(os.path.join(tmpdir, 'weighted_v.dat')) as fh:
                wc = len(fh.readlines())
            self.assertEqual(wc, 402)


if __name__ == '__main__':
    IMP.test.main()
