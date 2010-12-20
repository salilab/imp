import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs
import os
import time

class SAXSProfileTest(IMP.test.TestCase):

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
