import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):


    def test_envelope_fit_restraint(self):
        """Test EM EnvelopeFitRestraint restraint"""
        m = IMP.Model()

        #! read PDB
        mh = IMP.atom.read_pdb(self.get_input_file_name('complex-3d.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector(),
                               True, True)
        atoms = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)

        #! read map
        map = IMP.em.read_map(self.get_input_file_name('complex.mrc'),
                              IMP.em.MRCReaderWriter())

        #! determine density threshold
        resnum = len(IMP.atom.get_by_type(mh, IMP.atom.RESIDUE_TYPE))
        mass = IMP.atom.get_mass_from_number_of_residues(resnum)
        density_threshold = IMP.em.get_threshold_for_approximate_mass(map, 2.0*mass)
        print "density threshold: ", density_threshold

        #! init Restraint and evaluate
        efr = IMP.em.EnvelopeFitRestraint(atoms, map, density_threshold, 20.0)
        score = efr.unprotected_evaluate(None)
        print "score: ", score

        self.assertAlmostEqual(score, -0.65, delta=0.1)

if __name__ == '__main__':
    IMP.test.main()
