import IMP
import IMP.test
import IMP.em
import sys
import os
import unittest

class MRCWriteTest(IMP.test.TestCase):
    """Test MRC write function on sampled map"""
    def setUp(self):
        """ create particles """
        IMP.test.TestCase.setUp(self)
        self.particles = IMP.em.ParticlesProvider()
        self.particles.append(9.,5.,5.,1.,1.)
        self.particles.append(12.,9.,4.,1.,1.)
        self.particles.append(4.,5.,5.,1.,1.)

        resolution = 2.0
        voxel_size = 1.0
        em_map = IMP.em.SampledDensityMap(self.particles,
                                          resolution,voxel_size)
        em_map.calcRMS()
        self.rms = em_map.get_header().rms
        print "RMSD of original map = " + str(self.rms)
        self.erw = IMP.em.MRCReaderWriter()
        self.out_filename = "aa.mrc"
        IMP.em.write_map(em_map, self.out_filename,self.erw)

    def test_read(self):
        """read map back in and check that rmsd is the same """
        em_map = IMP.em.DensityMap()
        print self.out_filename
        em_map = IMP.em.read_map(self.out_filename,self.erw)
        os.unlink(self.out_filename)
        em_map.calcRMS()
        rms2 = em_map.get_header().rms
        print "RMSD of file = " + str(rms2)
        self.assert_(abs(rms2 - self.rms) < 0.00001, "rms values differ!")


class ReadWriteMapsTests(IMP.test.TestCase):

    def test_em_read_write(self):
        """test em format read/write """
        in_filename = self.get_input_file_name("three_particles_in.em")
        out_filename = "three_particles_out.em"

        xrw = IMP.em.EMReaderWriter()

        scene = IMP.em.read_map(in_filename,xrw)
        header = scene.get_header()
        self.assertEqual(header.nx, 55)
        self.assertEqual(header.ny, 55)
        self.assertEqual(header.nz, 55)
        self.assertEqual(header.magic, 6)
        self.assertEqual(header.voltage, 0.)
        self.assertEqual(header.Cs, 0.)
        self.assertEqual(header.Objectpixelsize, 1.)
        self.assertInTolerance(scene.calcRMS(), 404.5, 1.0)
        IMP.em.write_map(scene, out_filename,xrw);

        scene2 = IMP.em.read_map(out_filename, xrw)
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
        in_filename =  self.get_input_file_name("1tdx_sampled.mrc")
        out_filename =  "1tdx_sampled_out.mrc"
        scene = IMP.em.DensityMap()
        mrc_rw = IMP.em.MRCReaderWriter()
        scene= IMP.em.read_map(in_filename,mrc_rw)

        # Check header size
        self.assertEqual(74,scene.get_header().nx)
        self.assertEqual(71,scene.get_header().ny)
        self.assertEqual(65,scene.get_header().nz)
        print "rms: " + str(scene.calcRMS())
        self.assertInTolerance(scene.calcRMS(), 0.00688, 1.0)
        IMP.em.write_map(scene, out_filename,mrc_rw)
        os.unlink(out_filename)

    def test_emheader(self):
        """test correct I/O of EM header"""
        in_filename =  self.get_input_file_name("cube.em")
        print "read in "+in_filename + " ..."
        scene = IMP.em.DensityMap()
        em_rw = IMP.em.EMReaderWriter()
        scene= IMP.em.read_map(in_filename, em_rw)
        pixsize = scene.get_header().Objectpixelsize
        print "ObjectPixelsize = " + str(pixsize)
        self.assertInTolerance(3.0, scene.get_header().Objectpixelsize, 0.0001)
        self.assertInTolerance(300., scene.get_header().voltage, 0.0001)


if __name__ == '__main__':
    unittest.main()
