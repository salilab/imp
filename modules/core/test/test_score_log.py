from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.container


class Tests(IMP.test.TestCase):

    """Test distance restraint and three harmonic score functions"""

    def test_restraint_score(self):
        """Test restraint score log"""
        m = IMP.Model()
        r0 = IMP._ConstRestraint(m, [], 1)
        r0.set_name("r0")
        r1 = IMP._ConstRestraint(m, [], 2)
        r1.set_name("r1")
        sf = IMP.core.RestraintsScoringFunction([r0, r1])
        nm = self.get_tmp_file_name("score_log.csv")
        l = IMP.core.WriteRestraintScoresOptimizerState([r0, r1], nm)
        sf.evaluate(True)
        l.update()
        l.update()
        l.update()
        del l
        with open(nm, "r") as fh:
            txt = fh.read()
        print(txt)


if __name__ == '__main__':
    IMP.test.main()
