#test read/write em format
import sys,os,EM
import unittest



class test_sample_particles(unittest.TestCase):
	"""test particles sampling  """


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
