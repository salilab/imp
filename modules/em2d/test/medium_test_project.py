import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d as em2d
import IMP.em
import os

#- test generation of projections
#- even projections
#- noise
#

class Tests(IMP.test.TestCase):

    def test_random_projection_generation(self):
        """ Generation of random projection from a PDB file with em2d images"""
        testfile=self.get_input_file_name("opencv_test.spi")
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
        img=em2d.Image()
        img.set_size(rows,cols)
        srw = em2d.SpiderImageReaderWriter()
        rr=em2d.RegistrationResult()
        rr.set_random_registration(0,5)
        options = em2d.ProjectingOptions(apix, resolution)
        options.srw = srw
        em2d.get_projection(img,particles,rr,options)
        img.write(testfile,srw)
        self.assertTrue(os.path.isfile(testfile),
                        "Projection image not generated")
        os.remove(testfile)

    def test_even_projections(self):
        """ Evenly distributed em2d image projections from a PDB file"""
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        prot =  IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),
                                                                   smodel,ssel)
        IMP.atom.add_radii(prot)
        particles=IMP.core.get_leaves(prot)
        n_projections = 3
        rows = 80
        cols = 80
        resolution = 1
        apix = 1.5
        srw = em2d.SpiderImageReaderWriter()
        registration_values=em2d.get_evenly_distributed_registration_results(
                                                                  n_projections)
        options = em2d.ProjectingOptions(apix, resolution)
        projections = em2d.get_projections(particles,
                  registration_values,rows,cols,options)
        # Read the stored projections
        stored_projection_names= em2d.create_filenames(
                        n_projections,"1z5s-fast-projection","spi")
        for n in xrange(0,n_projections):
            stored_projection_names[n]=self.get_input_file_name(
                                            stored_projection_names[n])
        stored_projections=em2d.read_images(stored_projection_names,srw)
        # check
        for n in xrange(0,n_projections):
            for i in xrange(0,rows):
                for j in xrange(0,cols):
                    self.assertAlmostEqual(projections[n](i,j),
                           stored_projections[n](i,j), delta=0.001,
                           msg="Projections generated and stored are different")


    def test_noisy_projections(self):
        """ Test the generation of noisy projections"""
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        fn_model = self.get_input_file_name("1e6v.pdb")
        prot =IMP.atom.read_pdb(fn_model,smodel,ssel)
        particles=IMP.core.get_leaves(prot)
        n_projections = 16
        rows = 100
        cols = 100
        resolution = 1
        apix = 1.5
        noise_SNR = 0.5
        # read the stored noisy images
        stored_names=[]
        srw = em2d.SpiderImageReaderWriter()
        for i in xrange(0,n_projections):
            fn_subject = "1e6v-subject-%d-set-%d-%s-apix" \
                 "-%s-SNR.spi" % (i,n_projections,str(apix),str(noise_SNR))
            stored_names.append( self.get_input_file_name(fn_subject) )
        stored_images = em2d.read_images(stored_names,srw)

        # Read registration parameters and generate new images
        fn_regs = self.get_input_file_name('1e6v-subjects-0.5.params')
        Regs =em2d.read_registration_results(fn_regs)
        options = em2d.ProjectingOptions(apix, resolution)
        projections = em2d.get_projections(particles,Regs,
                               rows,cols,options)
        # Add noise
        for i in xrange(0,n_projections):
            em2d.do_normalize(projections[i],True)
            em2d.add_noise(projections[i],
                                0.0,1./(noise_SNR**0.5), "gaussian",3)
        # theoretical ccc for same images at a level of noise
        theoretical_ccc=noise_SNR/(noise_SNR+1)
        for n in xrange(0,n_projections):
            ccc=em2d.get_cross_correlation_coefficient(projections[n],
                                                      stored_images[n])
             # allow 3% difference in cross-correlation
            self.assertAlmostEqual(theoretical_ccc,ccc, delta=0.03,
                    msg="Noisy projections generated and stored are different")

if __name__ == '__main__':
    IMP.test.main()
