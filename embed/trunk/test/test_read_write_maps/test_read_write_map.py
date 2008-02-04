#test read/write em format
import sys,os,EM
import unittest

out_filename = os.path.abspath("data/aa.mrc")


class MRCWriteTest(unittest.TestCase):
    """Test MRC write function on sampled map"""
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
        print out_filename
        em_map.Read(out_filename,self.erw)
        os.unlink(out_filename)
        em_map.calcRMS()
        rms2 = em_map.get_header().rms
        print "RMSD of file = " + str(rms2)
        self.assert_(abs(rms2 - self.rms) < 0.00001, "rms values differ!")


class ReadWriteMapsTests(unittest.TestCase):
    def assertInTolerance(self, val, expected, tol, msg=None):
        if msg == None:
            msg = "%f differs from %f by more than %f" \
                  % (val, expected, tol)
        diff = abs(val - expected)
        self.assert_(diff < tol, msg)

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
        self.assertInTolerance(scene.calcRMS(), 404.5, 1.0)
        scene.Write(out_filename,xrw);

        scene2 = EM.DensityMap()
        scene2.Read(out_filename, xrw)
        header2 = scene2.get_header()
        self.assertEqual(header2.nx, header.nx)
        self.assertEqual(header2.ny, header.ny)
        self.assertEqual(header2.nz, header.nz)
        self.assertEqual(header2.magic, header.magic)
        self.assertInTolerance(scene2.calcRMS(), 404.5, 1.0)
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
        self.assertEqual(74,scene.get_header().nx)
        self.assertEqual(71,scene.get_header().ny)
        self.assertEqual(65,scene.get_header().nz)
        print "rms: " + str(scene.calcRMS())
        self.assertInTolerance(scene.calcRMS(), 0.00688, 1.0)
        scene.Write(out_filename,mrc_rw)
        os.unlink(out_filename)


if __name__ == '__main__':
    unittest.main()
