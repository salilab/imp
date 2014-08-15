import IMP.test
import IMP.atom
import sys
import re

rank_score = IMP.atom.get_data_path('protein_ligand_rank_score.lib')
pose_score = IMP.atom.get_data_path('protein_ligand_pose_score.lib')

class Tests(IMP.test.ApplicationTestCase):

    def get_inputs(self):
        return [self.get_input_file_name('1d3d-protein.pdb'),
                self.get_input_file_name('1d3d-ligands.mol2')]

    def test_rank_score(self):
        """Simple test of ligand score application with RankScore (default)"""
        for inputs in (self.get_inputs(), ['--rank'] + self.get_inputs(),
                       self.get_inputs() + [rank_score]):
            p = self.run_application('ligand_score', inputs)
            self.check_output(p, 8.39, 6.54)

    def test_pose_score(self):
        """Simple test of ligand score application with PoseScore"""
        for inputs in (['--pose'] + self.get_inputs(),
                       self.get_inputs() + [pose_score]):
            p = self.run_application('ligand_score', inputs)
            self.check_output(p, -27.78, -31.58)

    def test_bad_args(self):
        """Test bad argument combinations"""
        for inputs in (['--pose', '--rank'], ['--rank', rank_score]):
            p = self.run_application('ligand_score', inputs)
            self.assertNotEqual(p.wait(), 0)

    def check_output(self, p, val1, val2):
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        out = out.split('\n')
        for line, nligand, score in [(out[-3], '001', val1),
                                     (out[-2], '002', val2)]:
            m = re.match('Score for 1d3d\-ligand\.1d3d\.(\d+) is ([\d\.-]+)',
                         line)
            self.assertIsNotNone(m, msg="Score output not found in " + str(out))
            self.assertEqual(m.group(1), nligand)
            self.assertAlmostEqual(float(m.group(2)), score, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
