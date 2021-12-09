import os
import IMP
import IMP.test
import IMP.pmi.tools
import IMP.pmi.output
import IMP.rmf
import RMF

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
    def test_flatten(self):
        """Test _flatten function"""
        inp = [1,2,3,4]
        self.assertEqual(list(IMP.pmi.output._flatten(inp)), inp)
        inp = (1,2,(4,5,6),[3,4,(9,0),5],9)
        self.assertEqual(list(IMP.pmi.output._flatten(inp)),
                         [1,2,4,5,6,3,4,9,0,5,9])

    def test_multi_chainids(self):
        """Test multi-character chain IDs"""
        output = IMP.pmi.output.Output()
        c = output.multi_chainids
        self.assertEqual([c[i] for i in range(0, 4)],
                         ['A', 'B', 'C', 'D'])
        self.assertEqual([c[i] for i in range(24,28)],
                         ['Y', 'Z', 'AA', 'AB'])
        self.assertEqual([c[i] for i in range(50,54)],
                         ['AY', 'AZ', 'BA', 'BB'])
        self.assertEqual([c[i] for i in range(700,704)],
                         ['ZY', 'ZZ', 'AAA', 'AAB'])

    def test_pdb_names(self):
        """Test Output.get_pdb_names()"""
        m = IMP.Model()
        h1 = IMP.atom.Hierarchy(IMP.Particle(m))
        h2 = IMP.atom.Hierarchy(IMP.Particle(m))
        output = IMP.pmi.output.Output()
        output.init_pdb("test_output.pdb", h1)
        output.init_pdb("test_output2.pdb", h2)
        # Names are dict keys so come out unsorted
        self.assertEqual(sorted(output.get_pdb_names()),
                         ['test_output.pdb', 'test_output2.pdb'])
        os.unlink('test_output.pdb')
        os.unlink('test_output2.pdb')

    def test_get_particle_infos(self):
        """Test get_particle_infos_for_pdb_writing with no particles"""
        m = IMP.Model()
        empty_hier = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        output = IMP.pmi.output.Output()
        output.init_pdb("test_output.pdb", empty_hier)
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
        with IMP.allow_deprecated():
            self._check_stat_file(self.get_input_file_name(
                                                  "./output1/statv1.out"))

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
        self.assertAlmostEqual(numpy.average(numpy.array(vals).astype(float)),
                               10.1270600392)

        # Test filters and statistics
        stats = IMP.pmi.output.OutputStatistics()
        vals = po.get_fields(["AtomicXLRestraint"], get_every=3,
                             filtertuple=("AtomicXLRestraint", "<", 10.0),
                             statistics=stats)
        self.assertEqual(stats.total, 16)
        self.assertEqual(stats.passed_filterout, 16)
        self.assertEqual(stats.passed_get_every, 5)
        self.assertEqual(stats.passed_filtertuple, 3)

    def _check_coordinate_identity(self,ps1,ps2):
        for n,p in enumerate(ps1):
            d1=IMP.core.XYZ(p)
            d2=IMP.core.XYZ(ps2[n])
            self.assertEqual(IMP.core.get_distance(d1,d2),0)

    def test_get_best_models_provenance_no_filter(self):
        """Test provenance info added by get_best_models(), no filtering"""
        stat = self.get_input_file_name("./output1/stat.0.out")
        prov = []
        models = IMP.pmi.io.get_best_models([stat],
                        score_key='AtomicXLRestraint', provenance=prov)
        self.assertEqual(prov, [])

    def test_get_best_models_provenance(self):
        """Test provenance info added by get_best_models()"""
        stat1 = self.get_input_file_name("./output1/stat.0.out")
        stat2 = self.get_input_file_name("./output1/stat.1.out")
        prov = []
        models = IMP.pmi.io.get_best_models([stat1, stat2],
                        score_key='AtomicXLRestraint', get_every=2,
                        prefiltervalue=10.0, provenance=prov)
        self.assertEqual(len(prov), 3)
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.pmi.io.add_provenance(prov, [p])
        self.assertTrue(IMP.core.Provenanced.get_is_setup(p))
        prov = IMP.core.Provenanced(p).get_provenance()

        self.assertTrue(IMP.core.FilterProvenance.get_is_setup(prov))
        tp = IMP.core.FilterProvenance(prov)
        self.assertEqual(tp.get_method(), 'Total score')
        self.assertAlmostEqual(tp.get_threshold(), 10.0, delta=1e-4)
        self.assertEqual(tp.get_number_of_frames(), 12)
        prov = prov.get_previous()

        self.assertTrue(IMP.core.FilterProvenance.get_is_setup(prov))
        tp = IMP.core.FilterProvenance(prov)
        self.assertEqual(tp.get_method(), 'Keep fraction')
        self.assertAlmostEqual(tp.get_threshold(), 0.0, delta=1e-4)
        self.assertEqual(tp.get_number_of_frames(), 17)
        prov = prov.get_previous()

        self.assertTrue(IMP.core.CombineProvenance.get_is_setup(prov))
        tp = IMP.core.CombineProvenance(prov)
        self.assertEqual(tp.get_number_of_runs(), 2)
        self.assertEqual(tp.get_number_of_frames(), 33)

    def test_RMFHierarchyHandler(self):

        m=IMP.Model()
        rmf_name=self.get_input_file_name("output_test/rmfs/1.rmf3")
        rmfh=IMP.pmi.output.RMFHierarchyHandler(m,rmf_name)
        # test that it is indeed a hierarchy
        self.assertEqual(348,len(IMP.atom.get_leaves(rmfh)))
        s0=rmfh.get_children()[0]
        mols=s0.get_children()
        self.assertEqual(2,len(mols))
        names=[mol.get_name() for mol in mols]
        self.assertEqual(names[0],'Rpb4')
        self.assertEqual(names[1],'Rpb7')
        lvs=IMP.atom.get_leaves(rmfh)
        #test container

        #test_init_equality
        rmf_name_test=self.get_input_file_name("output_test/rmfs_test/1.rmf3")
        rh = RMF.open_rmf_file_read_only(rmf_name_test)
        h = IMP.rmf.create_hierarchies(rh, m)[0]
        IMP.rmf.load_frame(rh, RMF.FrameID(0))
        lvsh=IMP.atom.get_leaves(h)

        self._check_coordinate_identity(lvs,lvsh)
        #test_lenght:
        self.assertEqual(len(rmfh),rh.get_number_of_frames())

        #test_get_item
        rmfh[2]
        IMP.rmf.load_frame(rh, RMF.FrameID(2))
        self._check_coordinate_identity(lvs,lvsh)

        #test_iter
        for i in rmfh:
            IMP.rmf.load_frame(rh, RMF.FrameID(i))
            self._check_coordinate_identity(lvs,lvsh)

        #test_slice
        for i in rmfh[2:-1:3]:
            IMP.rmf.load_frame(rh, RMF.FrameID(i))
            self._check_coordinate_identity(lvs,lvsh)

        #test_linking
        rmf_name=self.get_input_file_name("output_test/rmfs/2.rmf3")
        rmfh.link_to_rmf(rmf_name)
        rmf_name_test=self.get_input_file_name("output_test/rmfs_test/2.rmf3")
        rh = RMF.open_rmf_file_read_only(rmf_name_test)
        IMP.rmf.link_hierarchies(rh, [h])
        IMP.rmf.load_frame(rh, RMF.FrameID(0))
        self._check_coordinate_identity(lvs,lvsh)
        self.assertEqual(len(rmfh),rh.get_number_of_frames())

    def test_StatHierarchyHandler_rmf_based(self):
        import ntpath
        import glob

        m=IMP.Model()
        stat_name=[self.get_input_file_name("output_test_rmf/rmfs/0.rmf3")]
        stath=IMP.pmi.output.StatHierarchyHandler(m,stat_name)

        # test that it is indeed a hierarchy
        self.assertEqual(348,len(IMP.atom.get_leaves(stath)))
        s0=stath.get_children()[0]
        mols=s0.get_children()
        self.assertEqual(2,len(mols))
        names=[mol.get_name() for mol in mols]
        self.assertEqual(names[0],'Rpb4')
        self.assertEqual(names[1],'Rpb7')
        lvs=IMP.atom.get_leaves(stath)

        #test internal data
        s1=stath[1].score
        r1=stath[1].rmf_name
        i1=stath[1].rmf_index
        s0=stath[0].score
        r0=stath[0].rmf_name
        i0=stath[0].rmf_index

        self.assertEqual(ntpath.basename(r1),"0.rmf3")
        self.assertEqual(ntpath.basename(r0),"0.rmf3")
        self.assertEqual(i1,1)
        self.assertEqual(i0,0)

        #test multiple stat files with filter
        stat_names=glob.glob(self.get_input_file_name("output_test_rmf/rmfs/0.rmf3").replace("0.rmf3","*.rmf3"))
        stath=IMP.pmi.output.StatHierarchyHandler(m,stat_names,5)

        lvs=IMP.atom.get_leaves(stath)
        self.assertEqual(798,len(stath))

    def test_plot_fields(self):
        """Test the plot_fields function"""
        try:
            import matplotlib
        except ImportError:
            self.skipTest("no matplotlib package")
        fs = {'score': [42., 32., 22., 11., 0.],
              'SwapSuccessRatio': [0.5, 0.4, 0.3, 0.2, 0.5]}
        IMP.pmi.output.plot_fields(fs, 'plot_fields_test1.png')
        os.unlink('plot_fields_test1.png')

        fs = {'score': [42., 32., 22., 11., 0.]}
        IMP.pmi.output.plot_fields(fs, 'plot_fields_test2.png',
                                   framemin=1, framemax=3)
        os.unlink('plot_fields_test2.png')

    def test_plot_field_histogram(self):
        """Test the plot_field_histogram function"""
        try:
            import matplotlib
        except ImportError:
            self.skipTest("no matplotlib package")
        scores = [[42., 32., 22., 11., 0.], [10., 20., 30., 40., 50.]]
        IMP.pmi.output.plot_field_histogram("scores_test", scores,
                                            yplotrange=[0,80.])
        os.unlink('scores_test.png')


if __name__ == '__main__':
    IMP.test.main()
