import IMP.test
import sys
import os
import re


class SAXSProfileApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple(self):
        """Simple test of SAXS profile application"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('foxs',
                                 ['-g',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'6lyz\.pdb.*Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.20, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt',
                    '6lyz.plt', '6lyz_lyzexp.fit'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_cif(self):
        """Simple test of SAXS profile application with mmCIF input"""
        p = self.run_application('foxs',
                                 ['-g',
                                  self.get_input_file_name('6lyz.cif'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'6lyz\.cif.*Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.20, delta=0.01)
        for out in ('6lyz.cif.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.plt',
                    '6lyz.plt', '6lyz_lyzexp.fit'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_cif_multimodel(self):
        """Test of SAXS profile application with multimodel mmCIF input"""
        p = self.run_application('foxs',
                                 ['-g', '-m', '2',
                                  self.get_input_file_name('6lyz.cif'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        # Should have three fits for the three models
        m = re.search(r'6lyz_m1.cif.*Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.20, delta=0.01)

        m = re.search(r'6lyz_m2.cif.*Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 149.65, delta=0.03)

        m = re.search(r'6lyz_m3.cif.*Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 134.08, delta=0.01)

        for model in ('_m1', '_m2', '_m3'):
            for out in ('6lyz%s.cif.dat' % model, '6lyz%s_lyzexp.dat' % model,
                        '6lyz%s_lyzexp.fit' % model,
                        '6lyz%s_lyzexp.plt' % model,
                        '6lyz%s.plt' % model):
                os.unlink(self.get_input_file_name(out))
        os.unlink('fit.plt')
        os.unlink('profiles.plt')

    def test_simple_exp_data_nm(self):
        """Simple test of SAXS profile application"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp_nm.dat'))
        p = self.run_application('foxs',
                                 ['-g',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp_nm.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.20, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp_nm.dat', '6lyz_lyzexp_nm.plt',
                    '6lyz.plt', '6lyz_lyzexp_nm.fit'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_no_fit(self):
        """Simple test of SAXS profile application no fitting of c1/c2 parameters"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('foxs',
                                 ['--min_c1', '1.0', '--max_c1', '1.0', '--min_c2', '0.0', '--max_c2', '0.0',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.289, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.fit'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_residue(self):
        """Simple test of SAXS profile application CA only calculation"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('foxs',
                                 ['-r',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.67, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.fit'):
            os.unlink(self.get_input_file_name(out))

    def test_simple_js(self):
        """Simple test of SAXS profile application js output"""
        print(self.get_input_file_name('6lyz.pdb'))
        print(self.get_input_file_name('lyzexp.dat'))
        p = self.run_application('foxs',
                                 ['-j',
                                  self.get_input_file_name('6lyz.pdb'),
                                  self.get_input_file_name('lyzexp.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'Chi\^2\s+=\s+([\d\.]+)\r?', out)
        self.assertIsNotNone(m, msg="Chi output not found in " + str(out))
        self.assertAlmostEqual(float(m.group(1)), 0.20, delta=0.01)
        for out in ('6lyz.pdb.dat', '6lyz_lyzexp.dat', '6lyz_lyzexp.fit'):
            os.unlink(self.get_input_file_name(out))

        with open('jmoltable.cif') as fh:
            cif_lines = fh.readlines()
        self.assertEqual(cif_lines[0].rstrip('\r\n'), 'loop_')
        self.assertEqual(len(cif_lines), 1020)
        for out in ('jmoltable.cif', 'jmoltable.html', 'canvas.plt'):
            os.unlink(out)

if __name__ == '__main__':
    IMP.test.main()
