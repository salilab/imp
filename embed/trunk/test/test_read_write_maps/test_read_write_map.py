#test read/write em format
import sys,os,EM
import unittest

out_filename = os.path.abspath("data/aa.mrc")



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


class test_write_mrc(unittest.TestCase):
	"""test mrc write function on sampled map  """
	def setUp(self):
           """ create particles """
           self.particles_p = particles_provider()
           self.particles_p.Init()
           resolution = 2.0
	   voxel_size = 1.0
	   em_map = EM.SampledDensityMap(self.particles_p,
					 resolution,voxel_size)
	   em_map.calcRMS()
	   self.rms = em_map.get_header().rms
	   print "RMSD of original map = " + str(self.rms)
	   self.erw = EM.MRCReaderWriter()
	   em_map.Write(out_filename,self.erw)

	def test_read(self):
		"""read map back in and check that rmsd is the same """
		em_map = EM.DensityMap()
		em_map.Read(out_filename,self.erw)
		em_map.calcRMS()
		rms2 = em_map.get_header().rms
		print "RMSD of file = " + str(rms2)
		self.assert_(abs(rms2 - self.rms) < 0.00001, "rms values differ!")

	
class test_read_write_maps(unittest.TestCase):

	def test_em_read_write(self):
	    """test em format read/write """
	    in_filename = os.path.abspath("data/three_particles_in.em")
	    out_filename = os.path.abspath("data/three_particles_out.em")

	    scene = EM.DensityMap()
	    xrw = EM.EMReaderWriter()

	    scene.Read(in_filename,xrw)
	    scene.Write(out_filename,xrw);

	#todo - add assert functions

	def test_mrc_read_write(self):
	        """test mrc format read/write """
		in_filename =  os.path.abspath("data/1tdx_sampled.mrc")
		out_filename =  os.path.abspath("data/1tdx_sampled_out.mrc")
		scene = EM.DensityMap()
		mrc_rw = EM.MRCReaderWriter()
		scene.Read(in_filename,mrc_rw)            
            
		# Check header size
		self.assertEqual(68,scene.get_header().nx)
		self.assertEqual(65,scene.get_header().ny)
		self.assertEqual(59,scene.get_header().nz)
		scene.Write(out_filename,mrc_rw)
	

if __name__ == '__main__':
    print 'start test_read_write_map'
    unittest.main()
    print 'pass test_read_write_map'
