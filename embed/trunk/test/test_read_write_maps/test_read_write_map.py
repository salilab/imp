#test read/write em format
import sys,os,EM
import unittest

out_filename = os.path.abspath("data/aa.mrc")



# #implement the particle access point class
# class particles_provider( EM.ParticlesAccessPoint):
#        def Init(self):
#                self.x_=[9.0,12.0,4.0]
#                self.y_=[5.0,9.0,5.0]
#                self.z_=[5.0,4.0,5.0]
#                self.w_=[1.0,1.0,1.0]
#                self.r_=[1.0,1.0,1.0]
#                self.size_=3

#        def get_size(self):
#                return self.size_;
#        def get_x(self,ind):
#                return self.x_[ind];
#        def get_y(self,ind):
#                return self.y_[ind];

#        def get_z(self,ind):
#                return self.z_[ind];

#        def get_r(self,ind):
#                return self.r_[ind];

#        def get_w(self,ind):
#                return self.w_[ind];


class test_write_mrc(unittest.TestCase):
	"""test mrc write function on sampled map  """
	def setUp(self):
           """ create particles """
           self.particles = EM.particles_provider()
           self.particles.append(9.,5.,5.,1.,1.)
           self.particles.append(12.,9.,4.,1.,1.)
           self.particles.append(4.,5.,5.,1.,1.)

           resolution = 2.0
	   voxel_size = 1.0
	   em_map = EM.SampledDensityMap(self.particles,
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
                os.unlink(out_filename)
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
            header = scene.get_header()
            self.assertEqual(header.nx, 55)
            self.assertEqual(header.ny, 55)
            self.assertEqual(header.nz, 55)
            self.assertEqual(header.magic, 6)
            self.assertEqual(header.voltage, 0.)
            self.assertEqual(header.Cs, 0.)
            self.assertEqual(header.Objectpixelsize, 1.)
	    scene.Write(out_filename,xrw);
            os.unlink(out_filename)

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
                os.unlink(out_filename)
	

if __name__ == '__main__':
    print 'start test_read_write_map'
    unittest.main()
    print 'pass test_read_write_map'
