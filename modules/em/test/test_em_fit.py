import unittest
import os
import IMP
import EM
import IMP.em
import IMP.test
import IMP.utils
import IMP.core

class FittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        self.scene = EM.DensityMap()
        erw = EM.EMReaderWriter()
        self.scene.Read(self.get_input_file_name("in.em"), erw)
        self.scene.get_header_writable().set_resolution(3.)
        header = self.scene.get_header()
        self.assertEqual(header.nx, 27)
        self.assertEqual(header.ny, 27)
        self.assertEqual(header.nz, 27)
    def load_particles(self):
        self.radius_key = IMP.FloatKey("radius")
        self.weight_key = IMP.FloatKey("weight")
        protein_key = IMP.FloatKey("protein")
        id_key = IMP.FloatKey("id")


        self.particles = []
        origin =  3.0
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         9.+origin, 9.+origin,
                                                         9.+origin))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         12.+origin, 3.+origin,
                                                         3.+origin))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         3.+origin, 12.+origin,
                                                         12.+origin))
        p1 = self.particles[0]
        p1.add_attribute(self.radius_key, 1.0)
        p1.add_attribute(self.weight_key, 1.0)
        p1.add_attribute(protein_key, 1)
        p1.add_attribute(id_key, 1)

        p1 = self.particles[1]
        p1.add_attribute(self.radius_key, 1.0)
        p1.add_attribute(self.weight_key, 1.0)
        p1.add_attribute(protein_key, 1)
        p1.add_attribute(id_key, 2)

        p1 = self.particles[2]
        p1.add_attribute(self.radius_key, 1.0)
        p1.add_attribute(self.weight_key, 1.0)
        p1.add_attribute(protein_key, 1)
        p1.add_attribute(id_key, 3)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()

        self.restraint_sets = []
        self.rsrs = []

        self.load_density_map()
        self.load_particles()

        self.opt = IMP.core.ConjugateGradients()

    def test_load_nonexistent_file(self):
        """Check that load of nonexistent file is handled cleanly"""
#        scene = EM.DensityMap()
#        erw = EM.EMReaderWriter()
#        self.assertRaises(IOError, scene.Read, "/not/exist/foo", erw)

    def test_em_fit(self):
        """Check that correlation of particles with their own density is 1"""
        r = IMP.em.EMFitRestraint(self.particles,self.scene,
                                  self.radius_key,
                                  self.weight_key, 1.0)
        self.imp_model.add_restraint(r)
        score = self.imp_model.evaluate(False)
        print "EM score (1-CC) = "+str(score)
        self.assert_(score < 0.05, "the correlation score is not correct")

    def test_cast(self):
        """Make sure that we can cast Restraint* to EMFitRestraint*"""
        m = self.imp_model
        r1 = IMP.em.EMFitRestraint(self.particles,self.scene,
                                   IMP.FloatKey("radius"),
                                   IMP.FloatKey("weight"), 1.0)
        sf = IMP.core.Harmonic(10.0, 0.1)
        r2 = IMP.core.DistanceRestraint(sf, self.particles[0],
                                        self.particles[1])
        m.add_restraint(r1)
        m.add_restraint(r2)
        r1 = m.get_restraint(0)
        r2 = m.get_restraint(1)
        self.assert_(isinstance(IMP.em.EMFitRestraint.cast(r1),
                                IMP.em.EMFitRestraint))
        self.assert_(IMP.em.EMFitRestraint.cast(r2) is None)
    def test_get_methods(self):
        """Check EMFitRestraint's get methods"""
        r1 = IMP.em.EMFitRestraint(self.particles,self.scene,
                                   self.radius_key,
                                   self.weight_key, 1.0)
        r1.set_was_owned(True)
        self.assert_(isinstance(r1.get_model_dens_map(), EM.SampledDensityMap))


if __name__ == '__main__':
    unittest.main()
