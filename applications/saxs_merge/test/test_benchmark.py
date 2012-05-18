#!/usr/bin/python

import IMP.test
import os
import sys

class SAXSApplicationTest(IMP.test.ApplicationTestCase):
    def test_case1(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        p = self.run_python_application('saxs_merge.py',
                          ['--creference=first',
                          self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
                          self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
                          self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
                          self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
                          self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        fin = open('data_merged.dat', 'r')
        text = fin.read()
        fin.close()
        fin = open('mean_merged.dat', 'r')
        text = fin.read()
        fin.close()
        fin = open('summary.txt', 'r')
        text = fin.read()
        fin.close()
        m = text.find('Classification')
        self.assertIsNotNone(m, msg="Classification output not found in summary.txt")

        os.unlink('data_merged.dat')
        os.unlink('mean_merged.dat')
        os.unlink('summary.txt')

    def test_case2(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        p = self.run_python_application('saxs_merge.py',
                          ['--creference=first',
                          self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
                          self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
                          self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
                          self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
                          self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        fin = open('data_merged.dat', 'r')
        text = fin.read()
        fin.close()
        fin = open('mean_merged.dat', 'r')
        text = fin.read()
        fin.close()
        fin = open('summary.txt', 'r')
        text = fin.read()
        fin.close()
        m = text.find('Classification')
        self.assertIsNotNone(m, msg="Classification output not found in summary.txt")

        os.unlink('data_merged.dat')
        os.unlink('mean_merged.dat')
        os.unlink('summary.txt')

if __name__ == '__main__':
    IMP.test.main()
