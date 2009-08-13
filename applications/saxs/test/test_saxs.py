import unittest
import IMP.test
import sys
import os
import re

class SAXSProfileApplicationTest(IMP.test.ApplicationTestCase):
    def test_simple(self):
        """Simple test of SAXS profile application"""
        p = self.run_application('profile',
                                 [self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        p.stdin.close()
        out = p.stdout.readlines()
        err = p.stderr.read()
        ret = p.wait()
        sys.stdout.write(err)
        self.assertEqual(ret, 0)
        m = re.match('Chi\s+=\s+([\d\.]+)$', out[-1])
        self.assertNotEqual(m, None, msg="Chi output not found in " + str(out))
        self.assertInTolerance(float(m.group(1)), 0.539, 0.001)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt'):
            os.unlink(self.get_input_file_name(out))

if __name__ == '__main__':
    unittest.main()
