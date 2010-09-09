import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os

#- test generation of projections
#- even projections
#- noise
#

class ProjectTests(IMP.test.TestCase):
    def test_generate_a_random_projection(self):
        """ Generation of random projection from a PDB file"""
        testfile=self.get_input_file_name("test.spi")
        if(os.path.isfile(testfile)):
            # delete the file to check
            os.remove(testfile)
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        prot =  IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"), smodel,ssel)
        IMP.atom.add_radii(prot)
        particles=IMP.core.get_leaves(prot)
        rows = 80
        cols = 80
        resolution = 1
        apix = 1.5
        img=IMP.em.Image()
        img.resize(rows,cols)
        srw = IMP.em.SpiderImageReaderWriter()
        rr=IMP.em2d.RegistrationResult()
        rr.set_random_registration(0,5)
        IMP.em2d.generate_projection(img,particles,rr,resolution,apix,srw)
        img.write_to_floats(testfile,srw)
        self.assert_(os.path.isfile(testfile),"Projection image not generated")
        os.remove(testfile)

    def test_generation_of_even_projections(self):
        """ Generation of evenly distributed projections from a PDB file"""
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        prot =  IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"), smodel,ssel)
        IMP.atom.add_radii(prot)
        particles=IMP.core.get_leaves(prot)
        n_projections = 3
        rows = 80
        cols = 80
        resolution = 1
        apix = 1.5
        srw = IMP.em.SpiderImageReaderWriter()
        registration_values=IMP.em2d.evenly_distributed_registration_results(
                                                                  n_projections)
        projections = IMP.em2d.generate_projections(particles,registration_values,
                               rows,cols,resolution,apix,srw)
        # Read the stored projections
        stored_projection_names= IMP.em2d.generate_filenames(
                                      n_projections,"1z5s-projection","spi")
        for n in xrange(0,n_projections):
            stored_projection_names[n]=self.get_input_file_name(
                                            stored_projection_names[n])
        stored_projections=IMP.em.read_images(stored_projection_names,srw)
        # check
        for n in xrange(0,n_projections):
            for i in xrange(0,rows):
                for j in xrange(0,cols):
                    self.assertInTolerance(projections[n](i,j),
                                           stored_projections[n](i,j),0.001,
                                    "Projections generated and stored are different")

    def test_noisy_projections(self):
        """ Test the generation of noisy projections"""
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        prot =  IMP.atom.read_pdb(self.get_input_file_name("1e6v.pdb"), smodel,ssel)
        IMP.atom.add_radii(prot)
        particles=IMP.core.get_leaves(prot)
        n_projections = 16
        rows = 100
        cols = 100
        resolution = 1
        apix = 1.5
        noise_SNR = 0.5
        # read the noisy stored images
        stored_names=[]
        srw = IMP.em.SpiderImageReaderWriter()
        for i in xrange(0,n_projections):
            stored_names.append(self.get_input_file_name(
                "1e6v-subject-"+str(i)+"-set-"+str(n_projections)+\
                    "-"+str(apix)+"-apix-"+str(noise_SNR)+"-SNR.spi") )
        stored_images=IMP.em.read_images(stored_names,srw)

        # Read registration parameters and generate new images
        Regs =IMP.em2d.read_registration_results(
          self.get_input_file_name('1e6v-subjects-0.5.params') )
        projections = IMP.em2d.generate_projections(particles,Regs,
                               rows,cols,resolution,apix,srw)

        # Add noise
        for i in xrange(0,n_projections):
            IMP.em.normalize(projections[i])
            IMP.em.add_noise(projections[i].get_data(),
                              0.0,1./(noise_SNR**0.5), "gaussian")

        # theoretical ccc for same images at a level of noise
        theoretical_ccc=noise_SNR/(noise_SNR+1)
        # check
        for n in xrange(0,n_projections):
            matrix1=projections[n].get_data()
            matrix2=stored_images[n].get_data()
            ccc=matrix1.cross_correlation_coefficient(matrix2)
             # allow 3% difference in cross-correlation
            self.assertInTolerance(theoretical_ccc,ccc,0.03,
                          "Noisy projections generated and stored are different")

if __name__ == '__main__':
    unittest.main()
