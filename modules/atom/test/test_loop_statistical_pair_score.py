import IMP
import IMP.test
import IMP.atom
import IMP.container


class Tests(IMP.test.TestCase):

    """Test the LoopStatisticalPairScore"""

    def test_value(self):
        """Check score value of LoopStatisticalPairScore"""
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_loop_statistical_score_data(mh)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        dpc = IMP.container.ClosePairContainer(ps, 7.0, 0.1)
        dps = IMP.atom.LoopStatisticalPairScore(7.0)
        d = IMP.container.PairsRestraint(dps, dpc)
        score = d.evaluate(False)
        self.assertAlmostEqual(score, 896.6609, delta=5.0)

if __name__ == '__main__':
    IMP.test.main()
