from __future__ import print_function
import IMP.test
import sys
import os
import re


class CrossLinksApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple_interface(self):
        """Simple test of interface cross links generation application"""
        p = self.run_application('interface_cross_links',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  '20'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # Check all identified cross links (distance <20)
        def parse_line(line):
            res1, res2, cid2, dist = line.split()
            return [int(res1), int(res2), cid2, float(dist)]
        all_cross_links = [parse_line(x) for x in out.split('\n')[:-1]]
        self.assertEqual(len(all_cross_links), 7)
        expected = [[60, 11, 'A', 19.2871],
                    [145, 1, 'A', 17.3919],
                    [145, 11, 'A', 19.5279],
                    [169, 1, 'A', 13.2192],
                    [188, 1, 'A', 17.5576],
                    [222, 1, 'A', 12.8005],
                    [224, 1, 'A', 8.94219]]
        for cl, excl in zip(all_cross_links, expected):
            self.assertEqual(cl[0], excl[0])
            self.assertEqual(cl[1], excl[1])
            self.assertEqual(cl[2], excl[2])
            self.assertAlmostEqual(cl[3], excl[3], delta=1e-2)

        # Check all selected cross links
        with open('cross_links.dat', 'r') as fin:
            sel_xl = fin.readlines()
        # Only the 3 crosslinks with dist < 15 are guaranteed to be selected;
        # the remainder are selected randomly
        self.assertGreaterEqual(len(sel_xl), 3)
        self.assertLessEqual(len(sel_xl), 7)
        self.assertIn('169 - 1 A 3 27 1\n', sel_xl)
        self.assertIn('224 - 1 A 3 27 1\n', sel_xl)
        self.assertIn('222 - 1 A 3 27 1\n', sel_xl)
        os.unlink('cross_links.dat')
        os.unlink('cxms2.dat')
        os.unlink('cxms_all.dat')

    def test_interface_version(self):
        """Test --version of interface cross links generation application"""
        p = self.run_application('interface_cross_links', ['--version'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Version:', err)

    def test_simple_single_structure_score(self):
        """Simple test of interface cross links single structure score
           application"""
        with open('complex.pdb', 'w') as destination:
            with open(self.get_input_file_name('static.pdb'), 'r') as fh:
                destination.write(fh.read())
            with open(self.get_input_file_name('transformed.pdb'), 'r') as fh:
                destination.write(fh.read())
        p = self.run_application('cross_links_single_score',
                                 ['complex.pdb',
                                  self.get_input_file_name('cxms.dat')])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search(r'Score\s+=\s+([\d\.]+)\r?', err)
        self.assertIsNotNone(m, msg="Score output not found in " + str(err))
        self.assertAlmostEqual(float(m.group(1)), 0.267, delta=0.01)

        os.unlink('complex.pdb')

    def test_cross_links_single_score_version(self):
        """Test --version of cross_links_single_score"""
        p = self.run_application('cross_links_single_score', ['--version'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Version:', err)

    def test_simple_score(self):
        """Simple test of cross linking score application"""
        p = self.run_application('cross_links_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name(
                                      'transforms-foxs10'),
                                  self.get_input_file_name('cxms.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        with open('cxms_score.res', 'r') as fin:
            text = fin.read()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 15)
        os.unlink('cxms_score.res')

    def test_cross_links_score_version(self):
        """Test --version of cross_links_score"""
        p = self.run_application('cross_links_score', ['--version'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Version:', err)


if __name__ == '__main__':
    IMP.test.main()
