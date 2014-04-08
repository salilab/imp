import IMP
import os
import IMP.test
import IMP.em
import IMP.multifit
import IMP.atom
from IMP.multifit import align


class Tests(IMP.test.TestCase):

    def test_align_help(self):
        """Test align module help"""
        self.check_runnable_python_module("IMP.multifit.align")

    def test_align_usage(self):
        """Test align module incorrect usage"""
        r = self.run_python_module("IMP.multifit.align", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_align_run(self):
        """Test align module run"""
        old_align = IMP.multifit.ProteomicsEMAlignmentAtomic

        class DummyAlignment(old_align):

            def set_density_map(self, dmap, threshold):
                pass

            def add_states_and_filters(self):
                pass

            def add_all_restraints(self):
                pass

            def align(self):
                pass

            def get_combinations(self):
                return []
        try:
            IMP.multifit.ProteomicsEMAlignmentAtomic = DummyAlignment
            self.assertRaises(SystemExit, self.run_python_module, align,
                              ['-m', '5', self.get_input_file_name('align.asmb.input'),
                               self.get_input_file_name(
                                   'align.proteomics.input'),
                                  self.get_input_file_name(
                                      'align.indexes.input'),
                                  self.get_input_file_name(
                                      'align.alignment.param'),
                                  'combinations.output', 'scores.output'])
            os.unlink('combinations.output')
        finally:
            IMP.multifit.ProteomicsEMAlignmentAtomic = old_align

    def test_align_report_solutions(self):
        """Test align module report_solutions function"""
        prot_data = IMP.multifit.read_proteomics_data(
            self.get_input_file_name('align.proteomics.input'))
        mapping_data = IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                                 self.get_input_file_name('align.indexes.input'))

        asmb = IMP.multifit.read_settings(
            self.get_input_file_name('align.asmb.input'))
        asmb.set_was_used(True)
        mdl = IMP.kernel.Model()
        mhs = []
        mhs.append(IMP.atom.read_pdb(self.get_input_file_name('twoblobsA.pdb'),
                                     mdl))
        IMP.atom.create_rigid_body(mhs[-1])
        mhs.append(IMP.atom.read_pdb(self.get_input_file_name('twoblobsB.pdb'),
                                     mdl))
        IMP.atom.create_rigid_body(mhs[-1])
        combs = [[0, 0], [1, 0], [2, 0], [3, 1]]

        old_fitr = IMP.em.FitRestraint
        old_ensmb = IMP.multifit.Ensemble
        old_add_rsr = IMP.kernel.Model.add_restraint

        class DummyEnsemble(old_ensmb):

            def load_combination(self, comb):
                pass

            def unload_combination(self, comb):
                pass

        class DummyFitRestraint(object):

            def __init__(self, leaves, dmap):
                self.scores = [1.0, 0.0, 0.5, 0.2]

            def evaluate(self, deriv):
                return self.scores.pop()

        def dummy_add_rsr(*args):
            pass

        try:
            IMP.em.FitRestraint = DummyFitRestraint
            IMP.multifit.Ensemble = DummyEnsemble
            IMP.kernel.Model.add_restraint = dummy_add_rsr

            align.report_solutions(asmb, mdl, mhs, None, mapping_data, combs,
                                   'test.comb.out', 'test.scores.out', 3)
        finally:
            IMP.em.FitRestraint = old_fitr
            IMP.multifit.Ensemble = old_ensmb
            IMP.kernel.Model.add_restraint = old_add_rsr
        lines = open('test.comb.out').readlines()
        lines = [x.rstrip(' \r\n') for x in lines]
        self.assertEqual(lines, ['2 0', '0 0', '1 0'])
        lines = open('test.scores.out').readlines()
        lines = [x.rstrip(' \r\n') for x in lines]
        self.assertEqual(lines, ['|2 0 |1.0|', '|0 0 |0.8|', '|1 0 |0.5|'])
        os.unlink('test.comb.out')
        os.unlink('test.scores.out')

if __name__ == '__main__':
    IMP.test.main()
