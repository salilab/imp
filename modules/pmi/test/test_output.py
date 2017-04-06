import os
import IMP
import IMP.test
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output

output1_categories = ['AtomicXLRestraint', 'AtomicXLRestraint_0_BestDist',
        'AtomicXLRestraint_0_Prob', 'AtomicXLRestraint_0_Sig1',
        'AtomicXLRestraint_0_Sig2', 'AtomicXLRestraint_1_BestDist',
        'AtomicXLRestraint_1_Prob', 'AtomicXLRestraint_1_Sig1',
        'AtomicXLRestraint_1_Sig2', 'AtomicXLRestraint_NumViol',
        'AtomicXLRestraint_sig_high', 'AtomicXLRestraint_sig_low',
        'CHARMM_BONDS', 'CHARMM_BONDS_2', 'CHARMM_NONBONDED',
        'CHARMM_NONBONDED_2', 'MolecularDynamics_KineticEnergy',
        'ReplicaExchange_CurrentTemp', 'ReplicaExchange_MaxTempFrequency',
        'ReplicaExchange_MinTempFrequency', 'ReplicaExchange_SwapSuccessRatio',
        'SimplifiedModel_Total_Score', 'Stopwatch_None_delta_seconds',
        'rmf_file', 'rmf_frame_index']

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
        """Test reading stat file (v2)"""
        self._check_stat_file(self.get_input_file_name("./output1/stat.0.out"))

    def test_process_output_v1(self):
        """Test reading stat file (v1)"""
        self._check_stat_file(self.get_input_file_name("./output1/statv1.out"))

    def test_process_output_complex(self):
        """Test reading a more complex stat file"""
        fname = self.get_input_file_name("output/stat.2.out")
        po = IMP.pmi.output.ProcessOutput(fname)
        categories = sorted(po.get_keys())
        self.assertEqual(len(categories), 409)
        self.assertEqual(categories[4],
                  'ISDCrossLinkMS_Distance_interrb-'
                  'State:0-1004:med5_1076:med5-1-1-1.0_DSS')

    def _check_stat_file(self, fname):
        import numpy
        po = IMP.pmi.output.ProcessOutput(fname)

        categories = po.get_keys()
        self.assertEqual(sorted(categories), output1_categories)

        vals = po.get_fields(["AtomicXLRestraint"])["AtomicXLRestraint"]
        self.assertAlmostEqual(numpy.average(numpy.array(vals).astype(numpy.float)), 10.1270600392)

if __name__ == '__main__':
    IMP.test.main()
