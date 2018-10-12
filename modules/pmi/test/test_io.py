import IMP
import IMP.test
import IMP.pmi
import IMP.pmi.io
import os

class Tests(IMP.test.TestCase):

    def test_parse_dssp_single_chain(self):
        """Test reading DSSP files, single chain"""
        sses = IMP.pmi.io.parse_dssp(self.get_input_file_name('chainA.dssp'),
                                     'A')
        self.assertEqual(sorted(sses.keys()),sorted(['helix','beta','loop']))
        self.assertEqual(sses['helix'][1][0],[100,126,'A'])
        self.assertEqual(sses['beta'][0],[[76,78,'A'],[91,93,'A']])
        self.assertEqual(len(sses['helix']),20)
        self.assertEqual(len(sses['beta']),3)
        self.assertEqual(len(sses['loop']),32)

    def test_parse_dssp_multiple_chain(self):
        """Test reading DSSP files, single chain"""
        sses = IMP.pmi.io.parse_dssp(self.get_input_file_name('chainA.dssp'),
                                     name_map={'A':'prot1'})
        self.assertEqual(sorted(sses.keys()),sorted(['helix','beta','loop']))
        self.assertEqual(sses['helix'][1][0],[100,126,'prot1'])
        self.assertEqual(sses['helix'][-1][0], [443, 444, 'I'])
        self.assertEqual(sses['beta'][0],[[76,78,'prot1'],[91,93,'prot1']])
        self.assertEqual(len(sses['helix']),121)
        self.assertEqual(len(sses['beta']),18)
        self.assertEqual(len(sses['loop']),183)

    def test_save_best_models(self):
        """Test save_best_models()"""
        m = IMP.Model()
        stat_files = [self.get_input_file_name('ministat.out')]
        feature_keys = ['ISDCrossLinkMS_Distance_interrb']
        score_key = "SimplifiedModel_Total_Score_None"

        IMP.pmi.io.save_best_models(m, './', stat_files,
                                    number_of_best_scoring_models=2,
                                    score_key=score_key,
                                    feature_keys=feature_keys)
        # This makes a (deprecated) v1 statfile
        with IMP.allow_deprecated():
            po = IMP.pmi.output.ProcessOutput('top_2.out')
        fields = po.get_fields([score_key])
        self.assertEqual(len(fields[score_key]), 2)
        self.assertAlmostEqual(float(fields[score_key][0]), 10.0, delta=0.1)
        os.unlink('top_2.rmf3')
        os.unlink('top_2.out')

    def test_get_trajectory_models(self):
        """Test get_trajectory_models()"""
        m = IMP.Model()
        stat_files = [self.get_input_file_name('ministat.out')]
        score_key = "SimplifiedModel_Total_Score_None"

        (rmfs, rmf_frames, scores) = IMP.pmi.io.get_trajectory_models(
                                            stat_files, score_key=score_key)
        self.assertEqual(len(rmfs), 3)
        self.assertEqual(rmf_frames, [0,0,0])
        self.assertEqual([int(x) for x in scores], [10, 20, 999])

if __name__ == '__main__':
    IMP.test.main()
