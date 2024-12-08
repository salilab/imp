import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs


class Tests(IMP.test.TestCase):

    def test_weighted_profile(self):
        """Check profile fitting with weights"""
        m = IMP.Model()

        # read PDB
        mp1 = IMP.atom.read_pdb(self.get_input_file_name('4cln.pdb'), m,
                                IMP.atom.NonWaterNonHydrogenPDBSelector(),
                                True, True)

        particles1 = IMP.atom.get_by_type(mp1, IMP.atom.ATOM_TYPE)

        mp2 = IMP.atom.read_pdb(self.get_input_file_name('2bbm.pdb'), m,
                                IMP.atom.NonWaterNonHydrogenPDBSelector(),
                                True, True)

        particles2 = IMP.atom.get_by_type(mp2, IMP.atom.ATOM_TYPE)

        # add radius for water layer computation
        ft = IMP.saxs.get_default_form_factor_table()
        for i in range(0, len(particles1)):
            radius = ft.get_radius(particles1[i])
            IMP.core.XYZR.setup_particle(particles1[i], radius)
        for i in range(0, len(particles2)):
            radius = ft.get_radius(particles2[i])
            IMP.core.XYZR.setup_particle(particles2[i], radius)

        # compute surface accessibility
        s = IMP.saxs.SolventAccessibleSurface()
        surface_area1 = s.get_solvent_accessibility(IMP.core.XYZRs(particles1))
        surface_area2 = s.get_solvent_accessibility(IMP.core.XYZRs(particles2))

        # calculate SAXS profiles
        delta_q = 0.5 / 500
        model_profile1 = IMP.saxs.Profile(0.0, 0.5, delta_q)
        model_profile1.calculate_profile_partial(particles1, surface_area1)
        model_profile2 = IMP.saxs.Profile(0.0, 0.5, delta_q)
        model_profile2.calculate_profile_partial(particles2, surface_area2)

        # read the weighted profile
        weighted_profile = IMP.saxs.Profile(
            self.get_input_file_name('weighted.dat'))

        # resample model profiles into a weighted profile
        resampled_profile1 = IMP.saxs.Profile()
        model_profile1.resample(weighted_profile, resampled_profile1)
        resampled_profile2 = IMP.saxs.Profile()
        model_profile2.resample(weighted_profile, resampled_profile2)

        saxs_score = IMP.saxs.WeightedProfileFitterChi(weighted_profile)

        profile_list = [resampled_profile1, resampled_profile2]
        # Get score for single profile
        score, weights = saxs_score.compute_score(profile_list[:1],
                                                  False, False)
        self.assertAlmostEqual(score, 81.765, delta=0.001)
        self.assertEqual(len(weights), 1)
        self.assertAlmostEqual(weights[0], 1.0, delta=0.001)

        # Get score for both profiles
        score, weights = saxs_score.compute_score(profile_list, False, False)
        self.assertAlmostEqual(score, 3.031, delta=0.001)
        self.assertEqual(len(weights), 2)
        self.assertAlmostEqual(weights[0], 0.297, delta=0.001)
        self.assertAlmostEqual(weights[1], 0.703, delta=0.001)

        wfp = saxs_score.fit_profile(profile_list, 0.95, 1.05, -2.0, 4.0)
        chi = wfp.get_chi_square()
        weights = wfp.get_weights()

        print('Chi = ', str(chi))
        print('weight1 = ', weights[0], ' weight2 = ', weights[1])

        self.assertAlmostEqual(chi, 2.95, delta=0.1)
        self.assertAlmostEqual(weights[0], 0.283, delta=0.1)
        self.assertAlmostEqual(weights[1], 0.716, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
