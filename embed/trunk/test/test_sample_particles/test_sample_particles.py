#test that impEM is able to sample IMP particles on a grid.



import unittest
import EM,os,sys

out_filename = os.path.abspath("sample_particles/aa.em")


class test_sample_particles(unittest.TestCase):
	"""test particles sampling  """

	def setUp(self):
		 print 'start setup'
		 """ create particles """
		 self.particles_p = particles_provider()
		 self.particles.append(9.,5.,5.,1.,1.)
		 self.particles.append(12.,9.,4.,1.,1.)
		 self.particles.append(4.,5.,5.,1.,1.)
		 print 'after setup'

	def test_sample(self):
		""" test that the sampling works """
		resolution = 2.0
		voxel_size = 1.0
		scene = EM.SampledDensityMap(self.particles_p,
					     resolution,voxel_size)
                
		scene.std_normalize()

		erw = EM.EMReaderWriter()
		scene.Write(out_filename,erw)


		# the mean and std are not correct
		self.assert_(scene.get_header().dmean == 0.0, "unexpected mean for the map")
		self.assert_(scene.get_header().rms == 1.0, "unexpected rms for the map")

if __name__ == '__main__':
    unittest.main()
		 
