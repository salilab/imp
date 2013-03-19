import IMP
import IMP.test
import sys
import IMP.em
import IMP.base
import os

class Tests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        IMP.base.set_check_level(IMP.base.NONE)
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = []
        ## -  create a set of three particles in imp
        npart = 3
        self.rad_key=IMP.FloatKey("radius")
        self.weight_key=IMP.FloatKey("weight")
        for i, (x,y,z) in enumerate(((9.0, 9.0, 9.0),
                                     (12.0, 3.0, 3.0),
                                     (3.0, 12.0, 12.0))):
            p = self.create_point_particle(self.imp_model, x,y,z)
            p.add_attribute(self.rad_key, 1.0, False)
            p.add_attribute(self.weight_key, 10.0)
            p.add_attribute(IMP.IntKey("id"), i)
            p.add_attribute(IMP.IntKey("protein"), 1)
            self.particles.append(p)
        self.particle_indexes = []
        for i in range(npart):
            self.particle_indexes.append(i)
        print "initialization done ..."

    def _test_sample_map(self):
        """Check that sampling particles works"""
        erw=IMP.em.EMReaderWriter()
        resolution=1.
        voxel_size=1.
        model_map = IMP.em.SampledDensityMap(self.particles, resolution, voxel_size,self.rad_key,self.weight_key)
        IMP.em.write_map(model_map,"a.em",erw)
        for p in self.particles:
            v=IMP.core.XYZ(p).get_coordinates()
            self.assertGreater(model_map.get_value(v), 0.6,
                         "map was not sampled correctly")
        model_map.calcRMS()
        mapfile = IMP.base.create_temporary_file_name("xxx.em")
        IMP.em.write_map(model_map, mapfile, erw)
        em_map = IMP.em.DensityMap()
        em_map= IMP.em.read_map(mapfile, erw)
        em_map.calcRMS()
        self.assertAlmostEqual(em_map.get_header().rms,
                               model_map.get_header().rms, delta=.000001,
                               msg="standard deviations of maps differ")
        os.unlink(mapfile)

    def test_sample_pdb(self):
        """Check that sampling particles works"""
        mrw=IMP.em.MRCReaderWriter()
        resolution=6.
        voxel_size=1.
        mh=IMP.atom.read_pdb(self.get_input_file_name("d1q3sa1.pdb"),self.imp_model,IMP.atom.CAlphaPDBSelector())
        IMP.atom.add_radii(mh)
        ps=IMP.atom.get_leaves(mh)
        #sample through a constructor
        model_map = IMP.em.SampledDensityMap(ps, resolution, voxel_size)
        model_map.calcRMS()
        IMP.em.write_map(model_map, "xxx.mrc",mrw)
        reloaded_model_map = IMP.em.DensityMap()
        reloaded_model_map= IMP.em.read_map("xxx.mrc",mrw)
        reloaded_model_map.calcRMS()
        self.assertLess(IMP.algebra.get_distance(reloaded_model_map.get_origin(),
                                          model_map.get_origin()), .001,
                     "the reloaded and the original map should have the same origin")
        self.assertAlmostEqual(reloaded_model_map.get_header().rms,
                               model_map.get_header().rms, delta=.001,
                               msg="standard deviations of maps differ")

        #now test sampling in a different way
        reloaded_model_map.get_header_writable().set_resolution(resolution)
        model_map2 = IMP.em.SampledDensityMap(reloaded_model_map.get_header())
        model_map2.set_particles(ps)
        model_map2.resample()
        model_map2.calcRMS()

        #IMP.em.write_map(model_map2, "yyy.mrc",mrw)
        reloaded_model_map.calcRMS()
        print model_map.get_header().rms, ": " , \
              reloaded_model_map.get_header().rms , " : " ,\
              model_map2.get_header().rms

        self.assertAlmostEqual(reloaded_model_map.get_header().rms,
                               model_map2.get_header().rms, delta=.001,
                               msg="standard deviations of maps differ")
        os.unlink("xxx.mrc")
        #os.unlink("yyy.mrc")


if __name__ == '__main__':
    IMP.test.main()
