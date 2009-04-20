import unittest
from particles_provider import ParticlesProvider
import IMP
import IMP.em
import IMP.test

class CrossCorrelationTests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # Initial values and names of files
        self.fn_in = self.get_input_file_name('1tdx_sampled.mrc')
        self.resolution=6.0
        self.fn_coords = self.get_input_file_name('1tdx_atoms')
        self.pixel_size=1.0


        self.EM_map = IMP.em.DensityMap()
        self.mrc_rw = IMP.em.MRCReaderWriter()
        self.EM_map.Read(self.fn_in,self.mrc_rw)
        self.EM_map.std_normalize()
        self.EM_map.get_header_writable().compute_xyz_top()

        self.atoms = ParticlesProvider()
        self.atoms.read(self.fn_coords)

        self.model_map = IMP.em.SampledDensityMap(self.atoms, self.resolution,
                                                  self.pixel_size)
        self.xo=self.model_map.get_header().get_xorigin()
        self.yo=self.model_map.get_header().get_yorigin()
        self.zo=self.model_map.get_header().get_zorigin()


        self.ccc = IMP.em.CoarseCC()
        self.ccc_intervals = IMP.em.CoarseCCatIntervals()


    def calc_simple_correlation(self):

        self.model_map.calcRMS();
        threshold=self.model_map.get_header().dmin
        return self.ccc.cross_correlation_coefficient(self.EM_map,self.model_map,threshold,True)

    def test_simple_correlation(self):
        """ test that the simple fast ccc function works """
        score = self.calc_simple_correlation()
        self.assertAlmostEqual(1.000,score,2)


    def test_origin_translation(self):
        """ test that translating either the map or the particles does not change the cc score"""


        # test_correlation_function
        # compute correlation translating the origin of the model map

        xm=3;ym=1;zm=-2
        self.model_map.set_origin(self.xo-xm,self.yo-ym,self.zo-zm)

        self.model_map.calcRMS();
        threshold=self.model_map.get_header().dmin

        self.EM_map.get_header_writable().compute_xyz_top()
        score1 = self.ccc.cross_correlation_coefficient(self.EM_map,self.model_map,threshold,True)


        #compute correlation translating the particles
        self.model_map.set_origin(self.xo,self.yo,self.zo)
        self.atoms.translate(xm,ym,zm)
        interval=1
        dvx = IMP.em.floats(); dvy= IMP.em.floats(); dvz= IMP.em.floats()
        score= self.ccc.evaluate(self.EM_map,self.model_map,self.atoms,dvx,dvy,dvz,1.0,False)
        score2 = 1.-score
        self.assertAlmostEqual(score1,score2,2)

    # Here we change the origin of the model map ( but not the particles position). We then resample the particles,
    # we made sure that all information is inside the translated map. Here we test that the correlation value does not change.
    def test_corr_consistency(self):
        """ test that two maps that sample the same particles have cc score of 1 """

        simple_score = self.calc_simple_correlation()

        xm=4;ym=-2;zm=0

        self.model_map.set_origin(self.xo-xm,self.yo-ym,self.zo-zm)
        interval=1
        dvx = IMP.em.floats(); dvy= IMP.em.floats(); dvz= IMP.em.floats()
        score= self.ccc.evaluate(self.EM_map,self.model_map,self.atoms,dvx,dvy,dvz,1.0,False)

        score=1.-score
        self.assertAlmostEqual(simple_score,score,2)

    # Check that the function works at intervals
    def test_corr_at_intervals(self):

        """ test that the correlation at intervals functionality works"""
        self.model_map.set_origin(self.xo,self.yo,self.zo)
        interval=5;         times=10;       scores_interval=[]
        dvx = IMP.em.floats(); dvy= IMP.em.floats(); dvz= IMP.em.floats()
        for d in [dvx,dvy,dvz]:
            for i in range(self.atoms.get_size()):
                d.push_back(0.0)
            for i in xrange(0,times):
                score=self.ccc_intervals.evaluate(self.EM_map,self.model_map,self.atoms,dvx,dvy,dvz,1.0,False,interval)
                scores_interval.append(score)
                self.atoms.translate(0.1,0.1,0.1)
        # check that the scores are equal when they have to be due to the function skipping computations
        for i in xrange(0,times):
            if(i%interval==0):
                result=scores_interval[i]
            self.assertEqual(result,scores_interval[i])


if __name__=='__main__':
    unittest.main()
