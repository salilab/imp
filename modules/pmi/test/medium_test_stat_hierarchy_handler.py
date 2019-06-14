import os
import IMP
import IMP.test
import IMP.pmi.tools
import IMP.pmi.output
import IMP.rmf
import RMF

class Tests(IMP.test.TestCase):

    def _check_data_identity(self,d1,d2):
        self.assertEqual(d1.rmf_name,d2.rmf_name)
        self.assertEqual(d1.rmf_index,d2.rmf_index)
        self.assertEqual(d1.score,d2.score)
        self.assertEqual(d1.stat_file,d2.stat_file)
        for k in d1.features:
            self.assertEqual(d1.features[k], d2.features[k])

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

    def _check_coordinate_identity(self,ps1,ps2):
        for n,p in enumerate(ps1):
            d1=IMP.core.XYZ(p)
            d2=IMP.core.XYZ(ps2[n])
            self.assertEqual(IMP.core.get_distance(d1,d2),0)

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
            self._check_data_identity(d,stath_read[n])
            self._check_coordinate_identity(lvs, lvs_read)


if __name__ == '__main__':
    IMP.test.main()
