import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):


    def test_envelope_fit_restraint(self):
        """Test EM EnvelopeFitRestraint restraint"""
        m = IMP.kernel.Model()

        #! read PDB
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector(),
                               True, True)
        atoms = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)

        #! read map
        map = IMP.em.read_map(self.get_input_file_name('mini-4.0.mrc'),
                              IMP.em.MRCReaderWriter())

        #! determine density threshold
        density_threshold = 0.04

        #! init Restraint and evaluate
        efr = IMP.em.EnvelopeFitRestraint(atoms, map, density_threshold, 4.0)
        efr.set_was_used(True)
        score = efr.unprotected_evaluate(None)

        self.assertAlmostEqual(score, -0.684, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
