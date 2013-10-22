import IMP.test
import sys

class PDBCheckApplicationTest(IMP.test.ApplicationTestCase):
    def test_simple(self):
        """Simple test of pdb_check application"""
        p = self.run_application('pdb_check',
                                 [self.get_input_file_name('mini.pdb')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        lines = out.split('\n')
        # In fast mode, warnings are not printed
        if len(lines) > 1:
            self.assertEqual(lines[0].rstrip('\r\n'), 'Atom not found "H"')

if __name__ == '__main__':
    IMP.test.main()
