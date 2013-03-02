#test that impEM is able to sample IMP particles on a grid.

import IMP
import IMP.test
import IMP.em
import os

class Tests(IMP.test.TestCase):
    """test particles sampling  """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        print 'start setup'
        """ create particles """
        self.particles = []
        self.mass_key=IMP.atom.Mass.get_mass_key()
        self.mdl=IMP.Model()
        for val in [[9.,5.,5.,1.,1.],[12.,9.,4.,1.,1.],[4.,5.,5.,1.,1.]]:
            p=IMP.Particle(self.mdl)
            IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(val[0],val[1],val[2]),val[3]))
            p.add_attribute(self.mass_key,val[4])
            self.particles.append(p)
        print 'after setup SampleParticlesTest'

    def test_sample(self):
        """ test that the sampling works """
        resolution = 2.0
        voxel_size = 1.0
        out_filename = "aa.em"

        scene = IMP.em.SampledDensityMap(self.particles,
                                         resolution,voxel_size,self.mass_key)

        scene.std_normalize()

        erw = IMP.em.EMReaderWriter()
        IMP.em.write_map(scene, out_filename, erw)
        # make sure that the output file is created
        os.unlink(out_filename)


        # the mean and std are not correct
        self.assertEqual(scene.get_header().dmean, 0.0,
                         "unexpected mean for the map")
        self.assertEqual(scene.get_header().rms, 1.0,
                         "unexpected rms for the map")

if __name__ == '__main__':
    IMP.test.main()
