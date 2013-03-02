import IMP
import IMP.em
import IMP.test

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        # Initial values and names of files
        self.fn_in = self.get_input_file_name('1tdx_sampled.mrc')
        self.resolution=6.0
        self.fn_coords = self.get_input_file_name('1tdx.pdb')
        self.pixel_size=1.0

        self.mrc_rw = IMP.em.MRCReaderWriter()
        self.EM_map = IMP.em.read_map(self.fn_in,self.mrc_rw)
        self.EM_map.std_normalize()
        self.EM_map.get_header_writable().compute_xyz_top()
        self.mdl=IMP.Model()
        mh=IMP.atom.read_pdb(self.fn_coords,self.mdl,IMP.atom.CAlphaPDBSelector())
        IMP.atom.add_radii(mh)
        self.atoms=IMP.core.get_leaves(mh)
        self.model_map = IMP.em.SampledDensityMap(self.atoms, self.resolution,
                                                  self.pixel_size)
        self.xo=self.model_map.get_header().get_xorigin()
        self.yo=self.model_map.get_header().get_yorigin()
        self.zo=self.model_map.get_header().get_zorigin()

        self.EM_map = IMP.em.SampledDensityMap(self.atoms, self.resolution,                                                                                                 self.pixel_size)
        self.EM_map.std_normalize()
        self.EM_map.get_header_writable().compute_xyz_top()

        self.ccc = IMP.em.CoarseCC()
        self.ccc_intervals = IMP.em.CoarseCCatIntervals()

    def calc_simple_correlation(self):

        self.model_map.calcRMS();
        threshold=self.model_map.get_header().dmin-0.1
        return self.ccc.cross_correlation_coefficient(self.EM_map,self.model_map,threshold,False)

    def test_simple_correlation(self):
        """ test that the simple fast ccc function works """
        score = self.calc_simple_correlation()
        self.assertAlmostEqual(1.000,score,2)


    def test_correlation_with_padding(self):
        """ test that padding option does not affect the CC score"""
        self.model_map.calcRMS()
        threshold=self.model_map.get_header().dmin-0.001
        score1 = self.ccc.cross_correlation_coefficient(self.EM_map,self.model_map,threshold,False)
        score2 = self.ccc.cross_correlation_coefficient(self.EM_map,self.model_map,threshold,True)
        self.assertAlmostEqual(score1,score2,2)


    def test_origin_translation(self):
        """ test that translating either the map or the particles does not change the cc score"""


        # test_correlation_function
        # compute correlation translating the origin of the model map

        xm=3;ym=1;zm=-2
        translation=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                                 IMP.algebra.Vector3D(xm,ym,zm))
        self.model_map.set_origin(self.xo-xm,self.yo-ym,self.zo-zm)

        self.model_map.calcRMS();
        threshold=self.model_map.get_header().dmin

        self.EM_map.get_header_writable().compute_xyz_top()
        score1 = self.ccc.cross_correlation_coefficient(self.EM_map,self.model_map,threshold,False)

        #compute correlation translating the particles
        self.model_map.set_origin(self.xo,self.yo,self.zo)
        for atom in self.atoms:
            IMP.core.XYZ(atom.get_particle()).set_coordinates(translation.get_transformed(IMP.core.XYZ(atom.get_particle()).get_coordinates()))
        interval=1
        score= self.ccc.calc_score(self.EM_map,self.model_map,1.0)
        score2 = 1.-score
        self.assertAlmostEqual(score1,score2, delta=.05*(score1+score2))

    # Here we change the origin of the model map ( but not the particles position). We then resample the particles,
    # we made sure that all information is inside the translated map. Here we test that the correlation value does not change.
    def test_corr_consistency(self):
        """ test that two maps that sample the same particles have cc score of 1 """

        simple_score = self.calc_simple_correlation()

        xm=4;ym=-2;zm=0

        self.model_map.set_origin(self.xo-xm,self.yo-ym,self.zo-zm)
        interval=1
        score= self.ccc.calc_score(self.EM_map,self.model_map,1.0)
        score=1.-score
        self.assertAlmostEqual(simple_score,score,2)

    # Check that the function works at intervals
    def test_corr_at_intervals(self):
        """ test that the correlation at intervals functionality works"""
        self.model_map.set_origin(self.xo,self.yo,self.zo)
        interval=5;         times=10;
        scores_intervals=[]
        scores_wo_intervals=[]
        dv = IMP.algebra.Vector3Ds()
        for i in range(len(self.atoms)):
            dv.append(IMP.algebra.Vector3D())
        translation=IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(0.1,0.1,0.1))
        #calculate correlation
        for i in xrange(0,times):
            scores_wo_intervals.append(self.ccc.calc_score(self.EM_map,self.model_map,1.0))
            scores_intervals.append(self.ccc_intervals.evaluate(self.EM_map,self.model_map,dv,1.0,False,interval))

            #transform the atoms
            for xyz in IMP.core.XYZs(self.atoms):
                xyz.set_coordinates(translation.get_transformed(xyz.get_coordinates()))
            # check that the scores are equal when they have to be due to the function skipping computations
        for i in xrange(0,times):
            if(i%interval==0):
                result=scores_intervals[i][0]
                self.assertAlmostEqual(scores_wo_intervals[i],
                                       scores_intervals[i][0],
                                       delta=1e-8)
            self.assertAlmostEqual(result, scores_intervals[i][0], delta=1e-8)

if __name__=='__main__':
    IMP.test.main()
