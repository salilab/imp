import os
import IMP
import IMP.test
import IMP.pmi.representation
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
        self.assertAlmostEqual(numpy.average(numpy.array(vals).astype(numpy.float)), 10.1270600392)

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

    def _check_data_indentity(self,d1,d2):
        self.assertEqual(d1.rmf_name,d2.rmf_name)
        self.assertEqual(d1.rmf_index,d2.rmf_index)
        self.assertEqual(d1.score,d2.score)
        self.assertEqual(d1.stat_file,d2.stat_file)
        for k in d1.features:
            self.assertEqual(d1.features[k], d2.features[k])

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

    def _get_info_from_stat_file(self, stat_file):
        po=IMP.pmi.output.ProcessOutput(stat_file)
        fs=po.get_keys()
        models = IMP.pmi.io.get_best_models([stat_file],
                                            score_key="Total_Score",
                                            feature_keys=fs,
                                            rmf_file_key="rmf_file",
                                            rmf_file_frame_key="rmf_frame_index",
                                            prefiltervalue=None,
                                            get_every=1)

        scores = [float(y) for y in models[2]]
        rmf_files = models[0]
        rmf_frame_indexes = models[1]
        features=models[3]
        return scores, rmf_files, rmf_frame_indexes,features


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

    def test_StatHierarchyHandler(self):
        import ntpath
        import glob

        m=IMP.Model()
        stat_name=[self.get_input_file_name("output_test/stat.1.out")]
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
        f1=stath[1].features['CrossLinkingMassSpectrometryRestraint_Distance_|0|0|0|XL|101.1|Rpb4|1|Rpb7|29|']
        s0=stath[0].score
        r0=stath[0].rmf_name
        i0=stath[0].rmf_index
        f0=stath[0].features['CrossLinkingMassSpectrometryRestraint_Distance_|0|0|0|XL|101.1|Rpb4|1|Rpb7|29|']

        self.assertEqual(ntpath.basename(r1),"1.rmf3")
        self.assertEqual(ntpath.basename(r0),"1.rmf3")
        self.assertEqual(i1,1)
        self.assertEqual(i0,0)
        self.assertNotEqual(s1,s0)
        self.assertNotEqual(f1,f0)

        #test __init__ equality
        rmf_name_test=self.get_input_file_name("output_test/rmfs_test/1.rmf3")
        rh = RMF.open_rmf_file_read_only(rmf_name_test)
        h = IMP.rmf.create_hierarchies(rh, m)[0]
        IMP.rmf.load_frame(rh, RMF.FrameID(0))
        lvsh=IMP.atom.get_leaves(h)
        self._check_coordinate_identity(lvs,lvsh)

        #test_lenght:
        self.assertEqual(len(stath),rh.get_number_of_frames())

        #test __getitem__
        stath[2]
        IMP.rmf.load_frame(rh, RMF.FrameID(2))
        self._check_coordinate_identity(lvs,lvsh)
        scores, rmf_files, rmf_frame_indexes,features=self._get_info_from_stat_file(stat_name[0])

        #test __iter__
        for n,o in enumerate(stath):
            IMP.rmf.load_frame(rh, RMF.FrameID(n))
            self._check_coordinate_identity(lvs,lvsh)
            self.assertEqual(o.rmf_index,n)
            self.assertEqual(ntpath.basename(o.rmf_name),"1.rmf3")
            self.assertEqual(o.score,scores[n])
            for k in o.features:
                self.assertEqual(o.features[k],features[k][n])

        #test data getters functionality
        self.assertEqual(stath.get_scores(),scores)
        self.assertEqual(stath.get_rmf_names(),rmf_files)
        self.assertEqual(stath.get_stat_files_names(),stat_name*len(stath))
        self.assertEqual(stath.get_rmf_indexes(),rmf_frame_indexes)
        self.assertEqual(sorted(stath.get_feature_names()),sorted(features.keys()))
        for k in features:
            self.assertEqual(stath.get_feature_series(k),features[k])

        #test score filtering
        stath.do_filter_by_score(7.5)
        tscores=[s for s in scores if s<7.5]
        self.assertEqual(stath.get_scores(),tscores)

        #test __iter__ score filtered
        for o in stath:
            IMP.rmf.load_frame(rh, RMF.FrameID(o.rmf_index))
            self._check_coordinate_identity(lvs,lvsh)
            self.assertEqual(ntpath.basename(o.rmf_name),"1.rmf3")
            n=rmf_frame_indexes.index(o.rmf_index)
            self.assertEqual(o.score,scores[n])
            for k in o.features:
                self.assertEqual(o.features[k],features[k][n])

        #test multiple stat files with slicing
        stat_names=glob.glob(self.get_input_file_name("output_test/stat.0.out").replace("stat.0.out","stat.*.out"))
        for stat in stat_names:
            stath.add_stat_file(stat)
        for o in stath[2:-1:10]:
            rh = RMF.open_rmf_file_read_only(o.rmf_name)
            IMP.rmf.link_hierarchies(rh, [h])
            IMP.rmf.load_frame(rh, RMF.FrameID(o.rmf_index))
            m.update()
            self._check_coordinate_identity(lvs,lvsh)

        #test multiple stat files with filter
        stath=IMP.pmi.output.StatHierarchyHandler(m,stat_names,10)
        lvs=IMP.atom.get_leaves(stath)
        self.assertEqual(10,len(stath))
        for o in stath:
            rh = RMF.open_rmf_file_read_only(o.rmf_name)
            IMP.rmf.link_hierarchies(rh, [h])
            IMP.rmf.load_frame(rh, RMF.FrameID(o.rmf_index))
            m.update()
            self._check_coordinate_identity(lvs,lvsh)

        #test copy constructor
        stathcopy=IMP.pmi.output.StatHierarchyHandler(StatHierarchyHandler=stath)
        lvsh=IMP.atom.get_leaves(stathcopy)
        self.assertEqual(stath.get_scores(),stathcopy.get_scores())
        self.assertEqual(stath.get_rmf_names(),stathcopy.get_rmf_names())
        self.assertEqual(stath.get_stat_files_names(),stathcopy.get_stat_files_names())
        self.assertEqual(stath.get_rmf_indexes(),stathcopy.get_rmf_indexes())
        self.assertEqual(sorted(stath.get_feature_names()),sorted(stathcopy.get_feature_names()))
        for o,ocopy in zip(stath,stathcopy):
            self._check_coordinate_identity(lvs,lvsh)
            for n,p in enumerate(lvs):
                self.assertNotEqual(p.get_particle_index(),lvsh[n].get_particle_index())

        #test save data
        stath.save_data('data.pkl')

        #test read data
        stath_read=IMP.pmi.output.StatHierarchyHandler(m,'data.pkl')
        lvs_read=IMP.atom.get_leaves(stath_read)

        for n,d  in enumerate(stath):
            self._check_data_indentity(d,stath_read[n])
            self._check_coordinate_identity(lvs, lvs_read)


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


if __name__ == '__main__':
    IMP.test.main()
