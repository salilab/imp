import IMP
import IMP.test
import IMP.em
import sys
import os

class Tests(IMP.test.TestCase):
    """Test MRC write function on sampled map"""
    def _setUp(self):
        """ create particles """
        IMP.test.TestCase.setUp(self)
        self.particles = []
        mdl=IMP.Model()
        self.weight_key=IMP.FloatKey("mass")
        for val in [[9.,5.,5.,1.,1.],[12.,9.,4.,1.,1.],[4.,5.,5.,1.,1.]]:
            p=IMP.Particle(mdl)
            IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(val[0],val[1],val[2]),val[3]))
            p.add_attribute(self.weight_key,5.)
            self.particles.append(p)
        resolution = 2.0
        voxel_size = 1.0
        em_map = IMP.em.SampledDensityMap(self.particles,
                                          resolution,voxel_size,
                                          self.weight_key)
        em_map.calcRMS()
        self.rms = em_map.get_header().rms
        print "RMSD of original map = " + str(self.rms)
        self.erw = IMP.em.MRCReaderWriter()
        self.out_filename = "aa.mrc"
        IMP.em.write_map(em_map, self.out_filename,self.erw)

    def _test_read(self):
        """read map back in and check that rmsd is the same """
        em_map = IMP.em.DensityMap()
        print self.out_filename
        em_map = IMP.em.read_map(self.out_filename,self.erw)
        os.unlink(self.out_filename)
        em_map.calcRMS()
        rms2 = em_map.get_header().rms
        print "RMSD of file = " + str(rms2)
        self.assertLess(abs(rms2 - self.rms), 0.00001, "rms values differ!")

    def _test_em_read_write(self):
        """test em format read/write """
        in_filename = self.get_input_file_name("three_particles_in.em")
        out_filename = "three_particles_out.em"

        xrw = IMP.em.EMReaderWriter()
        scene = IMP.em.read_map(in_filename,xrw)
        header = scene.get_header()
        self.assertEqual(header.get_nx(), 55)
        self.assertEqual(header.get_ny(), 55)
        self.assertEqual(header.get_nz(), 55)
        self.assertEqual(header.magic, 6)
        self.assertEqual(header.voltage, 0.)
        self.assertEqual(header.Cs, 0.)
        self.assertEqual(header.get_spacing(), 1.)
        self.assertAlmostEqual(scene.calcRMS(), 404.5, delta=1.0)
        IMP.em.write_map(scene, out_filename,xrw);
        scene2 = IMP.em.read_map(out_filename, xrw)
        header2 = scene2.get_header()
        self.assertEqual(header2.get_nx(), header.get_nx())
        self.assertEqual(header2.get_ny(), header.get_ny())
        self.assertEqual(header2.get_nz(), header.get_nz())
        self.assertEqual(header2.magic, header.magic)
        self.assertAlmostEqual(scene2.calcRMS(), 404.5, delta=1.0)
        os.unlink(out_filename)

    #todo - add assert functions

    def _test_mrc_read_write(self):
        """test mrc format read/write """
        in_filename =  self.get_input_file_name("1tdx_sampled.mrc")
        out_filename =  "1tdx_sampled_out.mrc"
        scene = IMP.em.DensityMap()
        mrc_rw = IMP.em.MRCReaderWriter()
        scene= IMP.em.read_map(in_filename,mrc_rw)
        # Check header size
        self.assertEqual(94,scene.get_header().get_nx())
        self.assertEqual(98,scene.get_header().get_ny())
        self.assertEqual(84,scene.get_header().get_nz())
        print "rms: " + str(scene.calcRMS())
        self.assertAlmostEqual(scene.calcRMS(), 0.00688, delta=1.0)
        IMP.em.write_map(scene, out_filename,mrc_rw)
        os.unlink(out_filename)

    def test_emheader(self):
        """test correct I/O of EM header"""
        in_filename =  self.get_input_file_name("cube.em")
        print "read in "+in_filename + " ..."
        print "here -------------"
        #scene = IMP.em.DensityMap()
        print "here --------11111111111111-----"
        '''
        em_rw = IMP.em.EMReaderWriter()
        scene= IMP.em.read_map(in_filename, em_rw)
        print "here -------22222222222222------"
        pixsize = scene.get_header().get_spacing()
        print "ObjectPixelsize = " + str(pixsize)
        print "here ------3333333333333-------"
        self.assertAlmostEqual(3.0, scene.get_header().get_spacing(),
                               delta=0.0001)
        self.assertAlmostEqual(300., scene.get_header().voltage, delta=0.0001)
        print "here -------------"
        '''

if __name__ == '__main__':
    IMP.test.main()
