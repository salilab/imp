#!/usr/bin/env python


######################################################
# PROGRAM
######################################################


import unittest

from particles_provider import particles_provider
from EM import MRCReaderWriter,vectorf
from EM import DensityMap, SampledDensityMap,CoarseCCatIntervals
import EM 


class cross_correlation_tests(unittest.TestCase):

	def setUp(self):
		# Initial values and names of files
	 	self.fn_in='data/1tdx_sampled.mrc'
	 	self.resolution=6.0
	 	self.fn_coords='data/1tdx_atoms'
		self.pixel_size=1.0


                self.EM_map=DensityMap()
                self.mrc_rw=MRCReaderWriter()
                self.EM_map.Read(self.fn_in,self.mrc_rw)
                self.EM_map.std_normalize()
                self.EM_map.get_header_writable().compute_xyz_top()
                
                self.atoms=particles_provider()
                self.atoms.read(self.fn_coords)
                                
                self.model_map = SampledDensityMap(self.atoms,self.resolution,self.pixel_size)
                self.xo=self.model_map.get_header().get_xorigin()
		self.yo=self.model_map.get_header().get_yorigin()
		self.zo=self.model_map.get_header().get_zorigin()

                
                self.ccc = EM.CoarseCC()
                self.ccc_intervals = EM.CoarseCCatIntervals(self.atoms.get_size())


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
            dvx = EM.vectorf();		dvy= EM.vectorf(); 		dvz= EM.vectorf()
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
		dvx = EM.vectorf();		dvy= EM.vectorf(); 		dvz= EM.vectorf()
		score= self.ccc.evaluate(self.EM_map,self.model_map,self.atoms,dvx,dvy,dvz,1.0,False)

		score=1.-score
		self.assertAlmostEqual(simple_score,score,2)

        # Check that the function works at intervals                
        def test_corr_at_intervals(self):

            """ test that the correlation at intervals functionality works"""
            self.model_map.set_origin(self.xo,self.yo,self.zo)
            interval=5; 	times=10; 	scores_interval=[]
            dvx = EM.vectorf();		dvy= EM.vectorf(); 		dvz= EM.vectorf()
            for d in [dvx,dvy,dvz]:
                d.insert(d.begin(),self.atoms.get_size(),0.0)
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
