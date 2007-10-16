#test that impEM is able to sample IMP particles on a grid.



import unittest
import EM,os




#implement the particle access point class
class particles_provider( EM.ParticlesAccessPoint):
	def Init(self):
		self.x_=[9.0,12.0,4.0]
		self.y_=[5.0,9.0,5.0]
		self.z_=[5.0,4.0,5.0]
		self.w_=[1.0,1.0,1.0]
		self.r_=[1.0,1.0,1.0]
		self.size_=3


	def get_size(self):
		return self.size_;
	def get_x(self,ind):
		return self.x_[ind];
	def get_y(self,ind):
		return self.y_[ind];

	def get_z(self,ind):
		return self.z_[ind];

	def get_r(self,ind):
		return self.r_[ind];

	def get_w(self,ind):
		return self.w_[ind];


class test_sample_particles(unittest.TestCase):
	"""test particles sampling  """

	def setUp(self):
		 """ create particles """
		 self.particles_p = particles_provider()
		 self.particles_p.Init()

	def test_sample(self):
		""" test that the create map is reasonable """
		resolution = 2.0
		voxel_size = 1.0

		scene = EM.SampledDensityMap(self.particles_p,
					     resolution,voxel_size)
		

		erw = EM.EMReaderWriter()
		scene.Write("aa.em",erw)

		# the mean and std are not correct
		self.assert_(scene.get_header().dmean == 0.0, "unexpected mean for the map")
		self.assert_(scene.get_header().rms == 1.0, "unexpected rms for the map")


if __name__ == '__main__':
    unittest.main()
		 
