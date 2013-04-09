#test that impEM is able to sample IMP particles on a grid.
import IMP
import IMP.test
import IMP.em
import os
import sys

class Tests(IMP.test.TestCase):
    """Test particles sampling"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        in_filename = self.get_input_file_name("three_particles_in.em")

        mrw = IMP.em.MRCReaderWriter()
        erw = IMP.em.EMReaderWriter()
        self.scene= IMP.em.read_map(in_filename,erw)

        self.scene.get_header_writable().resolution = 2.0
        self.scene.std_normalize()


        self.particles = []
        mdl=IMP.Model()
        self.mass_key=IMP.atom.Mass.get_mass_key()
        for val in [[9.,5.,5.,1.,1.],[12.,9.,4.,1.,1.],[4.,5.,5.,1.,1.]]:
            p=IMP.Particle(mdl)
            IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(val[0],val[1],val[2]),val[3]))
            p.add_attribute(self.mass_key,val[4])
            self.particles.append(p)
        self.particles_sampling = IMP.em.SampledDensityMap(
                                              self.particles, 2.0, 1.0,self.mass_key)


    def test_sample(self):
        """ test that the create map is reasonable """
        return
        dvx = IMP.Floats();
        dvx.insert(dvx.begin(),3,0.0)
        dvy= IMP.Floats()
        dvy.insert(dvy.begin(),3,0.0)
        dvz= IMP.Floats()
        dvz.insert(dvz.begin(),3,0.0)


        cc = IMP.em.CoarseCC()
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
        self.assertLess(score, 0.1, "unexpected cross correlation score")



if __name__ == '__main__':
    IMP.test.main()
