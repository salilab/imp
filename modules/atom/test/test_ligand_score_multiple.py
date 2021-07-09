import IMP.test
import IMP.atom
import sys
import re
import os

class Tests(IMP.test.ApplicationTestCase):

    def test_score_pdb(self):
        """Simple test of ligand_score_multiple application, PDB input"""
        self._run_score_test(cif=False)

    def test_score_cif(self):
        """Simple test of ligand_score_multiple application, mmCIF input"""
        self._run_score_test(cif=True)

    def _run_score_test(self, cif):
        ext = 'cif' if cif else 'pdb'
        # Make transforms file from PatchDock output
        with open(self.get_input_file_name('ligscore_test.res')) as fh:
            data = fh.readlines()
        with open('tr', 'w') as fh:
            for d in data[3:-2]:
                d = d.strip().split('|')
                fh.write(d[0]+' '+d[-1]+'\n')
        p = self.run_application('ligand_score_multiple',
                      [self.get_input_file_name('ligscore_test.%s' % ext),
                       self.get_input_file_name('2F2.mol2'), 'tr'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        with open('mol2_score.res') as fh:
            scores = [float(i.split()[-1]) for i in fh]
        self.assertEqual(len(scores), 5)
        for s, exps in zip(scores, (-0.5, 1.49, -1.06, -1.48, 0.5)):
            self.assertAlmostEqual(s, exps, delta=1e-4)
        os.unlink('tr')
        os.unlink('mol2_score.res')

    def test_help(self):
        """Test ligand_score_multiple --help"""
        p = self.run_application('ligand_score_multiple', ['--help'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Usage: <pdb|mmcif> <mol2>', out)

    def test_version(self):
        """Test ligand_score_multiple --version"""
        p = self.run_application('ligand_score_multiple', ['--version'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Version:', err)


if __name__ == '__main__':
    IMP.test.main()
