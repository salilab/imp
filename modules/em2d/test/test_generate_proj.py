import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d as em2d
import IMP.em
import os, sys, re



#- test generation of projections
#- even projections
#- noise
#


class Tests(IMP.test.IMP.test.ApplicationTestCase):

    def test_even_projections(self):
        """ Evenly distributed em2d image projections from a PDB file"""
        p = self.run_application('GeneratePerfectProjection',
                                 [self.get_input_file_name('complex-2d.pdb'),
                                  '5',
                                  self.get_input_file_name('image_1.pgm'),
                                  '3', '2.5']) ###PDBFile, Number of Projections, Input Image, Resolution, Apix

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertTrue(os.path.isfile("Projection-2.pgm"),"Projection image not generated")

        os.remove("Projection-0.pgm")
        os.remove("Projection-1.pgm")
        os.remove("Projection-2.pgm")
        os.remove("Projection-3.pgm")
        os.remove("Projection-4.pgm")

if __name__ == '__main__':
    IMP.test.main()
