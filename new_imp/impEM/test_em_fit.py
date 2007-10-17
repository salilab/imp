import unittest
import os
import IMP
import IMPEM
import EM
import IMP.test
import IMP.utils


class test_fitting(IMP.test.IMPTestCase):
    """Class to test pair connectivity restraints"""



    def load_density_map(self):
        self.scene = EM.DensityMap()
        erw = EM.EMReaderWriter()
        self.scene.Read("in.em",erw)
        self.scene.get_header_writable().resolution = 2.0


    def load_particles(self):
        self.particles = []
        self.particle_indexes = IMP.vectori()



        for p in range(3):
            self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                        0., 0., 0.))
        p1 = self.particles[0]
        p1.add_float("radius", 1.0, False)
        p1.add_float("weight", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 1)
        self.particle_indexes.push_back(1)

        p1 = self.particles[1]
        p1.add_float("radius", 1.0, False)
        p1.add_float("weight", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 2)
        self.particle_indexes.push_back(2)


        p1 = self.particles[2]
        p1.add_float("radius", 1.0, False)
        p1.add_float("weight", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 3)
        self.particle_indexes.push_back(3)



    def setUp(self):


        """Build test model and optimizer"""
        self.imp_model = IMP.Model()

        self.restraint_sets = []
        self.rsrs = []


        self.load_density_map()
        self.load_particles()

        self.opt = IMP.ConjugateGradients()

    def test_load_nonexistent_file(self):
        """Check that load of nonexistent file is handled cleanly"""
        scene = EM.DensityMap()
        erw = EM.EMReaderWriter()
        self.assertRaises(IOError, scene.Read, "/not/exist/foo", erw)

    def test_em_fit(self):
        """Test pair connectivity restraint.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins """
        rs = IMP.RestraintSet("em")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint(rs)

        # add connectivity restraints

        rsrs = []


        rsrs.append(IMPEM.EMFitRestraint(self.imp_model,
                                              self.particle_indexes,
                                              self.scene,
                                              "radius",
                                              "weight",
                                              1.0))

        score = rsrs[0].evaluate(False)
        self.assert_(score > 0.05, "the correlation score is not correct")


if __name__ == '__main__':
    unittest.main()
