import os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.restrainer

class Tests(IMP.test.TestCase):
    """Class to test simple em fit restraint"""

    def load_particles(self, m, coordinates):
        """Load particles"""
        self.particles = []
        self.ps_dist = []
        self.mhs = IMP.atom.Hierarchies()

        radius_key = IMP.FloatKey("radius")
        mass_key = IMP.FloatKey("mass")
        protein_key = IMP.FloatKey("protein")
        id_key = IMP.FloatKey("id")

        for pt in coordinates:
            p = self.create_point_particle(m, *pt)
            p.add_attribute(radius_key, 1.0)
            p.add_attribute(mass_key, 1.0)
            p.add_attribute(protein_key, 1.0)
            p.add_attribute(id_key, 1.0)
            mp = IMP.atom.Hierarchy.setup_particle(p)
            self.particles.append(p)
            self.mhs.append(mp)

        self.ps_dist.append(self.particles[0])
        self.ps_dist.append(self.particles[1])

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.imp_model = IMP.Model()

        origin =  3.0
        self.load_particles(self.imp_model,
                          [(9.+origin, 9.+origin, 9.+origin),
                           (12.+origin, 3.+origin, 3.+origin),
                           (3.+origin, 12.+origin, 12.+origin)])

        self.dmap = IMP.restrainer.load_em_density_map (
                        self.get_input_file_name("in.mrc"), 1.0, 3.0)


    def test_em_fit(self):
        """Check that correlation of particles with their own density is 1"""

        se = IMP.restrainer.create_simple_em_fit(self.mhs, self.dmap)
        r = se.get_restraint()
        self.imp_model.add_restraint(r)
        # Make sure that refcounting is working correctly
        # refs from Python, the SimpleEMFit object, and the Model
        self.assertEqual(r.get_ref_count(), 3)

        score = self.imp_model.evaluate(False)
        print "EM score (1-CC) = "+str(score)
        self.assertLess(score, 0.05, "the correlation score is not correct")

    def test_cast(self):
        """Make sure that we can cast Restraint* to FitRestraint*"""

        se = IMP.restrainer.create_simple_em_fit(self.mhs, self.dmap)
        sd = IMP.restrainer.create_simple_distance(self.ps_dist)

        r1 = se.get_restraint()
        r2 = sd.get_restraint()

        self.imp_model.add_restraint(r1)
        self.imp_model.add_restraint(r2)

        self.assertIsInstance(IMP.em.FitRestraint.get_from(r1),
                              IMP.em.FitRestraint)
        self.assertRaises(IMP.ValueException, IMP.em.FitRestraint.get_from, r2)

        r1 = self.imp_model.get_restraint(0)
        r2 = self.imp_model.get_restraint(1)

        self.assertIsInstance(IMP.em.FitRestraint.get_from(r1),
                              IMP.em.FitRestraint)
        self.assertRaises(IMP.ValueException, IMP.em.FitRestraint.get_from, r2)


    def test_methods(self):
        """Check SimpleEMFit's methods"""

        se = IMP.restrainer.create_simple_em_fit(self.mhs, self.dmap)
        r1 = se.get_restraint()
        self.imp_model.add_restraint(r1)

        r1.set_was_used(True)
        self.assertIsInstance(r1.get_model_dens_map(), IMP.em.SampledDensityMap)

        test_mrc = IMP.restrainer.load_em_density_map (
                        self.get_input_file_name("1z5s.mrc"), 1.0, 3.0)

        se2 = IMP.restrainer.create_simple_em_fit(self.mhs, test_mrc)
        r2 = se.get_restraint()
        self.imp_model.add_restraint(r2)
        r2.set_was_used(True)
        self.assertIsInstance(r2.get_model_dens_map(), IMP.em.SampledDensityMap)


if __name__ == '__main__':
    IMP.test.main()
