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
        p = self.run_application('create_single_particle_images',
                                 ['-i', self.get_input_file_name('complex-2d.pdb'),
                                  '-n', '5',
                                  '-r', '3.3',
                                  '-a', '1.2156',
                                  '-l', '256',
                                  '-o', 'Projection',
                                  '-s', '0.00001',
                                  '-p', 'Noiseless']) ###PDBFile, Number of Projections, Resolution, Apix, Image size, Output Name, Noise Level (StdDev SSNR). process
        
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
