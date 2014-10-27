import IMP.test
import sys
import os
import re


class SAXSProfileApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple(self):
        """Simple test of SAXS profile application"""
        print self.get_input_file_name('6lyz.pdb')
        print self.get_input_file_name('lyzexp.dat')
        p = self.run_application('foxs',
                                 [self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('Chi\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.44, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt', '6lyz.plt'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_no_fit(self):
        """Simple test of SAXS profile application no fitting of c1/c2 parameters"""
        print self.get_input_file_name('6lyz.pdb')
        print self.get_input_file_name('lyzexp.dat')
        p = self.run_application('foxs',
                                 ['-e', '1.0', '-w', '0.0',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('Chi\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.53, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt', '6lyz.plt'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_residue(self):
        """Simple test of SAXS profile application CA only calculation"""
        print self.get_input_file_name('6lyz.pdb')
        print self.get_input_file_name('lyzexp.dat')
        p = self.run_application('foxs',
                                 ['-r',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('Chi\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.82, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt', '6lyz.plt'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_js(self):
        """Simple test of SAXS profile application js output"""
        print self.get_input_file_name('6lyz.pdb')
        print self.get_input_file_name('lyzexp.dat')
        p = self.run_application('foxs',
                                 ['-j',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('Chi\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.44, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt', '6lyz.plt'):
            os.unlink(self.get_input_file_name(out))

        for out in ('jmoltable.pdb', 'jmoltable.html', 'canvas.plt'):
            os.unlink(out)

if __name__ == '__main__':
    IMP.test.main()
