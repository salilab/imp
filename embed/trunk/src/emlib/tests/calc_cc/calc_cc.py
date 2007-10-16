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
		self.scene = EM.DensityMap()
		erw = EM.EMReaderWriter()
		self.scene.Read("in.em",erw)
		self.scene.get_header_writable().resolution = 2.0
		self.particles = particles_provider()
		self.particles.Init()

		self.particles_sampling = EM.SampledDensityMap(
			self.particles,
			2.0,1.0
			)
		

		
	def test_sample(self):
		""" test that the create map is reasonable """

		dvx = EM.vectorf();
		dvx.insert(dvx.begin(),3,0.0)
		dvy= EM.vectorf()
		dvy.insert(dvy.begin(),3,0.0)
		dvz= EM.vectorf()
		dvz.insert(dvz.begin(),3,0.0)

	
		cc = EM.CoarseCC()
		score=cc.evaluate(
			self.scene,
			self.particles_sampling,
			self.particles,
			dvx,dvy,dvz,
			1.0,
			0
	
			)
		print score
		# the mean and std are not correct
		self.assert_(score < 0.01, "unexpected cross correlation score")

if __name__ == '__main__':
    unittest.main()
		 
