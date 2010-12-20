import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs
import os
import time

class SAXSProfileTest(IMP.test.TestCase):

    def test_saxs_profile(self):
        """Check protein profile computation"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                              IMP.atom.NonWaterNonHydrogenPDBSelector(),
                              True, True)

        #! read experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))

        print 'min_q = ' + str(exp_profile.get_min_q())
        print 'max_q = ' + str(exp_profile.get_max_q())
        print 'delta_q = ' + str(exp_profile.get_delta_q())

        #! select particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        #! calculate SAXS profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(particles)
        #model_profile.write_SAXS_file('i_single_protein_IMP.txt')

        #! calculate chi-square
        saxs_score = IMP.saxs.Score(exp_profile)
        chi = saxs_score.compute_chi_score(model_profile)
        print 'Chi = ' + str(chi)
        self.assertAlmostEqual(chi, 0.5, delta=0.1)

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

        #! calculate SAXS profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile(particles)

        #! calculate chi-square
        saxs_score = IMP.saxs.Score(exp_profile)
        chi = saxs_score.compute_chi_score(model_profile)
        print 'Chi = ' + str(chi)

        #! define restraint
        saxs_restraint = IMP.saxs.Restraint(particles, exp_profile)
        m.add_restraint(saxs_restraint)
        score = saxs_restraint.evaluate(False)
        print 'initial score = ' + str(score)

if __name__ == '__main__':
    IMP.test.main()
