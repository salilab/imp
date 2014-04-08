import IMP
import IMP.test
import IMP.atom

from math import *


class Test(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.kernel.Model()
        self.ref_scores = {}
        self.ref_scores["1h68.pdb"] = -36.4341363762
        self.ref_scores["1ors.pdb"] = -13.1726910037
        self.ref_scores["2nr9.pdb"] = -29.1148381963
        self.ref_scores["2v8n.pdb"] = -69.2013021885
        self.ref_scores["4ej4.pdb"] = -35.0643840765
        self.rst = {}
        for pdb in self.ref_scores:
            prot = IMP.atom.read_pdb(self.get_input_file_name(pdb),
                                     self.m, IMP.atom.CBetaPDBSelector())
            ps = IMP.atom.get_leaves(prot)
            ez = IMP.atom.EzRestraint(ps)
            self.m.add_restraint(ez)
            self.rst[pdb] = ez

    def test_Ez(self):
        for pdb in self.ref_scores:
            score = self.rst[pdb].evaluate(False)
            self.assertAlmostEqual(score, self.ref_scores[pdb])

if __name__ == '__main__':
    IMP.test.main()
