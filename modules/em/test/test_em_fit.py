import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        ### Note: This test used to work with in.em, we moved to mrc
        ### as it was not clear how to save origin in em format.
        ### Resolve with Frido.
        self.mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name("in.mrc"), self.mrw)
        self.scene.get_header_writable().set_resolution(3.)
        header = self.scene.get_header()
        self.assertEqual(header.get_nx(), 33)
        self.assertEqual(header.get_ny(), 33)
        self.assertEqual(header.get_nz(), 33)

    def load_particles(self):
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()

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
        IMP.base.set_log_level(IMP.base.VERBOSE)
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
        for p in self.particles:
            print "is rigid body?",IMP.core.RigidBody.particle_is_instance(p)
        r = IMP.em.FitRestraint(self.particles,self.scene)
        self.imp_model.add_restraint(r)
        score = self.imp_model.evaluate(False)
        print "EM score (1-CC) = "+str(score)
        self.assertLess(score, 0.05, "the correlation score is not correct")

    def test_cast(self):
        """Make sure that we can cast Restraint* to FitRestraint*"""
        m = self.imp_model
        r1 = IMP.em.FitRestraint(self.particles,self.scene)
        sf = IMP.core.Harmonic(10.0, 0.1)
        r2 = IMP.core.DistanceRestraint(sf, self.particles[0],
                                        self.particles[1])
        m.add_restraint(r1)
        m.add_restraint(r2)
        r1 = m.get_restraint(0)
        r2 = m.get_restraint(1)
        self.assertIsInstance(IMP.em.FitRestraint.cast(r1),
                              IMP.em.FitRestraint)
        self.assertIsNone(IMP.em.FitRestraint.cast(r2))
    def test_get_methods(self):
        """Check FitRestraint's get methods"""
        r1 = IMP.em.FitRestraint(self.particles,self.scene)
        self.assertIsInstance(r1.get_model_dens_map(),
                              IMP.em.SampledDensityMap)


if __name__ == '__main__':
    IMP.test.main()
