import os
import IMP
import IMP.test
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_get_particle_infos(self):
        """Test get_particle_infos_for_pdb_writing with no particles"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        output = IMP.pmi.output.Output()
        output.init_pdb("test_output.pdb", simo.prot)
        info, center = output.get_particle_infos_for_pdb_writing(
                                              "test_output.pdb")
        self.assertEqual(len(info), 0)
        self.assertAlmostEqual(center[0], 0., delta=1e-5)
        self.assertAlmostEqual(center[1], 0., delta=1e-5)
        self.assertAlmostEqual(center[2], 0., delta=1e-5)
        os.unlink('test_output.pdb')

    def test_process_output_v2(self):
        """test reading stat file (v2)"""
        self._check_stat_file(self.get_input_file_name("./output1/stat.0.out"),
                              25)

    def test_process_output_v1(self):
        """test reading stat file (v1)"""
        self._check_stat_file(self.get_input_file_name("./output1/statv1.out"),
                              24)

    def _check_stat_file(self, fname, num_categories):
        import numpy
        po = IMP.pmi.output.ProcessOutput(fname)

        categories = po.get_keys()
        self.assertEqual(len(categories), num_categories)

        criteria = [("rmf_frame_index", 5, "<")]
        self.assertEqual(len(po.return_models_satisfying_criteria(criteria)), 11)

        criteria = [("rmf_frame_index", 5, "<"), ('AtomicXLRestraint', 10.0, ">")]
        self.assertEqual(len(po.return_models_satisfying_criteria(criteria)), 4)

        vals = po.get_fields(["AtomicXLRestraint"])["AtomicXLRestraint"]
        self.assertAlmostEqual(numpy.average(numpy.array(vals).astype(numpy.float)), 10.1270600392)

if __name__ == '__main__':
    IMP.test.main()
