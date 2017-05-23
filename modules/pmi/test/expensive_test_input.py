from __future__ import print_function
import IMP.pmi
import IMP.pmi.io
import IMP.pmi.analysis
import IMP.test
import IMP.rmf
import RMF
import os

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.mdl=IMP.Model()
        input_dir=os.path.dirname(self.get_input_file_name('chainA.pdb'))
        self.stat_files = [os.path.join(input_dir,'output','stat.2.out'),
                           os.path.join(input_dir,'output','stat.3.out')]
        self.feature_keys = ['ISDCrossLinkMS_Distance_intrarb','ISDCrossLinkMS_Distance_interrb',
                             'ISDCrossLinkMS_Data_Score','SimplifiedModel_Linker_Score_None',
                             'ISDCrossLinkMS_Psi','ISDCrossLinkMS_Sigma','GaussianEMRestraint_None']
        self.score_key = "SimplifiedModel_Total_Score_None"

    def test_save_best_models(self):
        """Test function to collect top models into a single RMF file"""
        IMP.pmi.io.save_best_models(self.mdl,'./',self.stat_files,
                                    number_of_best_scoring_models=3,
                                    score_key=self.score_key,
                                    feature_keys=self.feature_keys)
        po = IMP.pmi.output.ProcessOutput('top_3.out')
        fields = po.get_fields([self.score_key])
        self.assertEqual(len(fields[self.score_key]),3)
        self.assertEqual(float(fields[self.score_key][0]),301.048975729)
        rh = RMF.open_rmf_file_read_only('top_3.rmf3')
        prots = IMP.rmf.create_hierarchies(rh,self.mdl)

        # testing first coordinate of med2 for each frame
        check_coords=[(26.2109, 61.2197, -16.9102),
                      (20.8669, 42.4836, -4.40971),
                      (25.741, 60.6429, -11.0356)]

        for i in range(3):
            IMP.rmf.load_frame(rh,RMF.FrameID(i))
            psdict = IMP.pmi.analysis.get_particles_at_resolution_one(prots[0])
            coord = IMP.core.XYZ(psdict['med2'][0]).get_coordinates()
            print(coord)
            self.assertLess(IMP.algebra.get_distance(coord,
                                   IMP.algebra.Vector3D(check_coords[i])), 1e-3)
        os.unlink('top_3.rmf3')
        os.unlink('top_3.out')


    def test_get_best_models(self):
        results = IMP.pmi.io.get_best_models(self.stat_files,
                                             self.score_key,
                                             self.feature_keys,
                                             prefiltervalue=305.0)

        rmf_file_list,rmf_file_frame_list,score_list,feature_keyword_list_dict=results
        self.assertEqual(len(rmf_file_list),8)
        self.assertEqual(len(rmf_file_frame_list),8)
        self.assertEqual(len(score_list),8)
        for k,l in feature_keyword_list_dict.iteritems():
            self.assertEqual(len(l),8)

    def test_read_coordinates_of_rmfs(self):
        results = IMP.pmi.io.get_best_models(self.stat_files,
                                             self.score_key,
                                             self.feature_keys,
                                             prefiltervalue=305.0)

        rmf_file_list,rmf_file_frame_list,score_list,feature_keyword_list_dict=results
        rmf_tuples = zip(score_list,
                         rmf_file_list,
                         rmf_file_frame_list,
                         range(len(score_list)),
                         range(len(score_list)))
        rmsdc={'med2':'med2'}
        got_coords = IMP.pmi.io.read_coordinates_of_rmfs(self.mdl,
                                                         rmf_tuples,
                                                         alignment_components=None,
                                                         rmsd_calculation_components=rmsdc)
        all_coordinates=got_coords[0]
        alignment_coordinates=got_coords[1]
        rmsd_coordinates=got_coords[2]
        rmf_file_name_index_dict=got_coords[3]
        all_rmf_file_names=got_coords[4]
        self.assertEqual(len(all_coordinates),8)
        self.assertEqual(len(alignment_coordinates),8)
        self.assertEqual(len(rmsd_coordinates),8)
        self.assertEqual(rmsd_coordinates[0].keys(),['med2'])
        self.assertEqual(cmp(alignment_coordinates,[{}]*8),0)

        # testing first coordinate of med2 for each frame
        check_coords=[[17.23349762, 27.99548721,-8.91260719],
                      [24.54545784, 59.26082993, -3.0899663 ],
                      [25.41869354, 63.82009125, -16.76820946],
                      [26.21087265, 61.21968079, -16.91023064],
                      [25.74103165, 60.64290619, -11.03559399],
                      [15.40375996, 61.21142197, -2.43177772],
                      [14.46688747, 61.47344208, -3.97651935],
                      [20.86691856, 42.48360443, -4.40970659]]
        for i in range(8):
            self.assertEqual(sorted(all_coordinates[i].keys()),
                             sorted(['med2', 'med3', 'med14', 'med15', 'med16', 'med5']))
            self.assertAlmostEqual(IMP.algebra.get_distance(all_coordinates[i]['med2'][0],
                                                            IMP.algebra.Vector3D(check_coords[i])),0.0)
            self.assertAlmostEqual(IMP.algebra.get_distance(rmsd_coordinates[i]['med2'][0],
                                                            IMP.algebra.Vector3D(check_coords[i])),0.0)

if __name__ == '__main__':
    IMP.test.main()
