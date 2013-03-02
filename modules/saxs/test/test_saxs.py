import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs
import os
import time
import StringIO

class Tests(IMP.test.TestCase):

    def test_saxs_profile(self):
        """Check protein profile computation"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                              IMP.atom.NonWaterNonHydrogenPDBSelector(),
                              True, True)

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        #print 'min_q = ' + str(exp_profile.get_min_q())
        #print 'max_q = ' + str(exp_profile.get_max_q())
        #print 'delta_q = ' + str(exp_profile.get_delta_q())

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        #! add radius for water layer computation
        ft = IMP.saxs.default_form_factor_table()
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
        #model_profile.write_SAXS_file('i_single_protein_IMP.txt')

        #! calculate chi-square
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
        chi = saxs_score.compute_score(model_profile, False, 'chi.dat')
        print 'Chi = ' + str(chi)
        self.assertAlmostEqual(chi, 0.54, delta=0.01)

        saxs_score_log = IMP.saxs.ProfileFitterChiLog(exp_profile)
        chi = saxs_score_log.compute_score(model_profile, False, 'chilog.dat')
        print 'Chi log = ' + str(chi)
        self.assertAlmostEqual(chi, 0.0347, delta=0.001)

        #! calculate SAXS profile that is good for c1/c2 fitting using calculate_profile_partial
        model_profile = IMP.saxs.Profile(0, max_q, delta_q)
        model_profile.calculate_profile_partial(particles, surface_area)

        fp = saxs_score.fit_profile(model_profile,
                                    0.95, 1.12, -2.0, 4.0, False, "chi_fit.dat")
        chi = fp.get_chi()
        print 'Chi after adjustment of excluded volume and water layer parameters = ' + str(chi)
        sio = StringIO.StringIO()
        fp.show(sio)
        self.assertAlmostEqual(chi, 0.45, delta=0.01)

        #! test interval chi
        chi = saxs_score.compute_score(model_profile, 0.0, 0.2);
        print 'Chi interval [0.0:0.2] = ' + str(chi)
        self.assertAlmostEqual(chi, 0.482, delta=0.01)

        #! test chi with log intensities
        chi = (saxs_score_log.fit_profile(model_profile,
                                          0.95, 1.12, -2.0, 4.0, False, "chilog_fit.dat")).get_chi()
        print 'ChiLog after adjustment of excluded volume and water layer parameters = ' + str(chi)
        self.assertAlmostEqual(chi, 0.0323, delta=0.001)

    def test_saxs_restraint(self):
        """Check saxs restraint"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                              IMP.atom.NonWaterNonHydrogenPDBSelector())

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        print 'Atomic level, number of particles ' + str(len(particles))

        #! calculate SAXS profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(particles)

        #! calculate chi-square
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
        chi = saxs_score.compute_score(model_profile)
        self.assertAlmostEqual(chi, 0.54, delta=0.01)

        #! define restraint
        saxs_restraint = IMP.saxs.Restraint(particles, exp_profile)
        m.add_restraint(saxs_restraint)
        score = saxs_restraint.evaluate(False)
        self.assertAlmostEqual(score, 0.54, delta=0.01)


    def test_saxs_residue_level_restraint(self):
        """Check residue level saxs restraint"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                              IMP.atom.CAlphaPDBSelector())

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        print 'Residue level, number of particles ' + str(len(particles))

        #! calculate SAXS profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(particles, IMP.saxs.CA_ATOMS)

        #! calculate chi-square
        saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
        chi = saxs_score.compute_score(model_profile)
        print 'Chi = ' + str(chi)


        #! define residue level restraint
        saxs_restraint = IMP.saxs.Restraint(particles, exp_profile, IMP.saxs.CA_ATOMS)
        m.add_restraint(saxs_restraint)
        score = saxs_restraint.evaluate(False)
        print 'initial score = ' + str(score)


if __name__ == '__main__':
    IMP.test.main()
