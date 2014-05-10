import IMP
import IMP.test
import IMP.atom
import IMP.em2d
import sys
import os


class EM2DFitRestraintTest(IMP.test.TestCase):

    def test_simple(self):
        """Simple test of EM2D single PDB score application"""
        m = IMP.kernel.Model()
        #! read PDB
        mp = IMP.atom.read_pdb(self.get_input_file_name('complex-2d.pdb'), m,
                               IMP.atom.NonWaterNonHydrogenPDBSelector())

        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        image_list = [self.get_input_file_name('image_1.pgm')]

        pca_fit_restraint = IMP.em2d.PCAFitRestraint(
            particles, image_list, 2.2, 10, 100)
        score = pca_fit_restraint.evaluate(False)
        print 'initial score = ' + str(score)
        self.assertAlmostEqual(score, 0.052, delta=0.01)
        os.unlink('images.pgm')


if __name__ == '__main__':
    IMP.test.main()
