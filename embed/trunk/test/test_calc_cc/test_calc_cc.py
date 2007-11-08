#test that impEM is able to sample IMP particles on a grid.



import unittest
import EM,os,sys




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
		in_filename = os.path.abspath("data/three_particles_in.em")

		self.scene = EM.DensityMap()
		mrw = EM.MRCReaderWriter()
		erw = EM.EMReaderWriter()
		self.scene.Read(in_filename,erw)

		self.scene.get_header_writable().resolution = 2.0
		self.scene.std_normalize()

		
		self.particles = particles_provider()
		self.particles.Init()

		self.particles_sampling = EM.SampledDensityMap(
			self.particles,
			2.0,1.0
			)

		
	def test_sample(self):
		""" test that the create map is reasonable """
		return 
		dvx = EM.vectorf();
		dvx.insert(dvx.begin(),3,0.0)
		dvy= EM.vectorf()
		dvy.insert(dvy.begin(),3,0.0)
		dvz= EM.vectorf()
		dvz.insert(dvz.begin(),3,0.0)


		cc = EM.CoarseCC()
		self.scene.get_header_writable().compute_xyz_top();
		print "em_origin: " + str(self.scene.get_header().get_xorigin()) + '  '+ str(self.scene.get_header().get_yorigin()) + '  ' + str(self.scene.get_header().get_zorigin()) + '\n'
		print "model_origin: " + str(self.particles_sampling.get_header().get_xorigin()) + '  '+ str(self.particles_sampling.get_header().get_yorigin()) + '  ' + str(self.particles_sampling.get_header().get_zorigin()) + '\n'

		score=cc.evaluate(
			self.scene,
			self.particles_sampling,
			self.particles,
			dvx,dvy,dvz,
			1.0,
			0
			)
		print 'test_sample ' + str(score)
		self.assert_(score < 0.1, "unexpected cross correlation score")

	

if __name__ == '__main__':
    print 'start test_calc_cc'		
    unittest.main()
    print 'end test_calc_cc'			
		 
