from __future__ import print_function
import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs
import os
import time
import io


class Tests(IMP.test.TestCase):

    def test_saxs_profile(self):
        """Check protein profile computation"""
        m = IMP.Model()

        #! read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector(),
                               True, True)

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        # print 'min_q = ' + str(exp_profile.get_min_q())
        # print 'max_q = ' + str(exp_profile.get_max_q())
        # print 'delta_q = ' + str(exp_profile.get_delta_q())

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        #! add radius for water layer computation
        ft = IMP.saxs.get_default_form_factor_table()
        for i in range(0, len(particles)):
            radius = ft.get_radius(particles[i])
            IMP.core.XYZR.setup_particle(particles[i], radius)

        # compute surface accessibility
        s = IMP.saxs.SolventAccessibleSurface()
        surface_area = s.get_solvent_accessibility(IMP.core.XYZRs(particles))

        #! calculate SAXS profile
        max_q = 0.5
        delta_q = 0.5 / 500
        model_profile = IMP.saxs.Profile(0, max_q, delta_q)
        model_profile.calculate_profile(particles)
        # model_profile.write_SAXS_file('i_single_protein_IMP.txt')

        #! calculate chi-square
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
        chi = saxs_score.compute_score(model_profile, False, 'chi.dat')
        print('Chi = ' + str(chi))
        self.assertAlmostEqual(chi, 0.2916, delta=0.01)

        saxs_score_log = IMP.saxs.ProfileFitterChiLog(exp_profile)
        chi = saxs_score_log.compute_score(model_profile, False, 'chilog.dat')
        print('Chi log = ' + str(chi))
        self.assertAlmostEqual(chi, 0.0347, delta=0.001)

        #! calculate SAXS profile that is good for c1/c2 fitting using calculate_profile_partial
        model_profile = IMP.saxs.Profile(0, max_q, delta_q)
        model_profile.calculate_profile_partial(particles, surface_area)

        fp = saxs_score.fit_profile(model_profile,
                                    0.95, 1.12, -2.0, 4.0, False, "chi_fit.dat")
        chi = fp.get_chi_square()
        print('Chi after adjustment of excluded volume and water layer parameters = ' + str(chi))
        sio = io.BytesIO()
        fp.show(sio)
        self.assertAlmostEqual(chi, 0.2025, delta=0.01)

        #! test chi with log intensities
        chi = (saxs_score_log.fit_profile(model_profile,
                                          0.95, 1.12, -2.0, 4.0, False, "chilog_fit.dat")).get_score()
        print('ChiLog after adjustment of excluded volume and water layer parameters = ' + str(chi))
        self.assertAlmostEqual(chi, 0.0323, delta=0.001)

        #! test RatioVolatilityScore
        vr_score = IMP.saxs.ProfileFitterRatioVolatility(exp_profile);
        vr = (vr_score.fit_profile(model_profile,
                                   0.95, 1.12, -2.0, 4.0, False, "vr_fit.dat")).get_score()
        print('RatioVolatilityScore after adjustment of excluded volume and water layer parameters = ' + str(vr))
        self.assertAlmostEqual(vr, 5.70, delta=0.01)

    def test_saxs_restraint(self):
        """Check saxs restraint"""
        m = IMP.Model()

        #! read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector())

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        print('Atomic level, number of particles ' + str(len(particles)))

        #! calculate SAXS profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(particles)

        #! calculate chi-square
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
        chi = saxs_score.compute_score(model_profile)
        self.assertAlmostEqual(chi, 0.2916, delta=0.01)

        #! define restraint
        saxs_restraint = IMP.saxs.Restraint(particles, exp_profile)
        score = saxs_restraint.evaluate(False)
        self.assertAlmostEqual(score, 0.2916, delta=0.01)

        s = saxs_restraint.get_static_info()
        s.set_was_used(True)
        self.assertEqual(s.get_number_of_string(), 2)
        self.assertEqual(s.get_string_key(0), "type")
        self.assertEqual(s.get_string_value(0), "IMP.isd.Restraint")
        self.assertEqual(s.get_string_key(1), "form factor type")
        self.assertEqual(s.get_string_value(1), "heavy atoms")

        self.assertEqual(s.get_number_of_filename(), 1)
        self.assertEqual(s.get_number_of_float(), 3)
        self.assertEqual(s.get_float_key(0), "min q")
        self.assertAlmostEqual(s.get_float_value(0), 0.04138, delta=1e-4)
        self.assertEqual(s.get_float_key(1), "max q")
        self.assertAlmostEqual(s.get_float_value(1), 0.49836, delta=1e-4)
        self.assertEqual(s.get_float_key(2), "delta q")
        self.assertAlmostEqual(s.get_float_value(2), 0.00233152, delta=1e-4)
        self.assertEqual(s.get_number_of_int(), 0)

    def test_saxs_residue_level_restraint(self):
        """Check residue level saxs restraint"""
        m = IMP.Model()

        #! read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                               IMP.atom.CAlphaPDBSelector())

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        print('Residue level, number of particles ' + str(len(particles)))

        #! calculate SAXS profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(particles, IMP.saxs.CA_ATOMS)

        #! calculate chi-square
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
        chi = saxs_score.compute_score(model_profile)
        print('Chi = ' + str(chi))

        #! define residue level restraint
        saxs_restraint = IMP.saxs.Restraint(
            particles,
            exp_profile,
            IMP.saxs.CA_ATOMS)
        score = saxs_restraint.evaluate(False)
        print('initial score = ' + str(score))

    def test_saxs_residue_particle_restraint(self):
        """Check residue level saxs restraint using
        IMP.saxs.RESIDUES particles. Needed for PMI
        hierarchies at resolution=1, which have no atom particles
        """
        m = IMP.Model()
        mdl = IMP.Model() # New model for residue particles only

        #! read PDB (only CA atoms)
        mp = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                               IMP.atom.CAlphaPDBSelector())

        # Get all CA particles
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        outhiers = []

        for ca in particles:
            residue = IMP.atom.Residue(ca.get_parent())
            rt = residue.get_residue_type()
            vol = IMP.atom.get_volume_from_residue_type(rt)
            mass = IMP.atom.get_mass(rt)


            # Create new particle in mdl and set up as a Residue
            rp1 = IMP.Particle(mdl)
            this_res = IMP.atom.Residue.setup_particle(rp1, rt, residue.get_index())

            # Add radius and shape information
            radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
            shape = IMP.algebra.Sphere3D(IMP.core.XYZ(ca).get_coordinates(),radius)
            rp1.set_name("Residue_%i"%residue.get_index())
            IMP.core.XYZR.setup_particle(rp1,shape)
            IMP.atom.Mass.setup_particle(rp1,mass)

            outhiers.append(this_res)

        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))
        saxs_particles = IMP.atom.Selection(outhiers).get_selected_particles()

        # Ensure the particles list is equal to the number of residues
        self.assertEqual(len(saxs_particles), 129)

        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(saxs_particles, IMP.saxs.RESIDUES)
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)

        self.assertAlmostEqual(saxs_score.compute_score(model_profile), 1.03, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
