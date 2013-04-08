import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d as em2d
import IMP.em
import os
from math import *


class Tests(IMP.test.TestCase):
    def test_registration(self):
        """Test the registration of 3 subjects from 1gyt.pdb at 0.5 SNR"""
        # Get model from PDB file
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        fn_model = self.get_input_file_name("1gyt.pdb")
        prot =  IMP.atom.read_pdb(fn_model,smodel,ssel)
        particles=IMP.core.get_leaves(prot)
        # Read subject images
        srw = em2d.SpiderImageReaderWriter()
        selection_file=self.get_input_file_name("1gyt-subjects-0.5-SNR.sel")
        images_to_read_names=em2d.read_selection_file(selection_file)
        for i in xrange(0,len(images_to_read_names)):
            images_to_read_names[i] = self.get_input_file_name(
                                                  images_to_read_names[i])
        subjects = em2d.read_images(images_to_read_names,srw)
        self.assertEqual(len(subjects),3,"Problem reading subject images")

        # Generate 20 evenly distributed projections from the PDB file
        n_projections = 20
        proj_params=em2d.get_evenly_distributed_registration_results(
                                                                n_projections)
        rows=128
        cols=128
        pixel_size = 1.5
        resolution = 1 # for generating projections, use a very high resolution
        options = em2d.ProjectingOptions(pixel_size, resolution)
        projections = em2d.get_projections(particles,proj_params,
                    rows,cols, options)
        self.assertEqual(len(projections),n_projections,
                                          "Problem generating projections")
        # Prepare registration
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        finder = em2d.ProjectionFinder()
        score_function = em2d.EM2DScore()


        params = em2d.Em2DRestraintParameters(pixel_size,
                                              resolution,n_projections)
        params.save_match_images = False
        params.coarse_registration_method = em2d.ALIGN2D_PREPROCESSING
        params.optimization_steps=30
        params.simplex_initial_length=0.1
        params.simplex_minimum_size=0.01

        finder.setup(score_function, params)
        finder.set_model_particles(particles)
        finder.set_subjects(subjects)
        finder.set_projections(projections)
        finder.set_fast_mode(2)
        finder.get_complete_registration()
        # Recover the registration results:
        registration_parameters=finder.get_registration_results()
        fn_registration_results=self.get_input_file_name(
                                                "my_1gyt_registration.params")
        em2d.write_registration_results(fn_registration_results,
                                          registration_parameters)
        # Read the correct registration results:
        correct_parameters=em2d.read_registration_results(
        self.get_input_file_name("1gyt-subjects-0.5-SNR.params") )

        print "determined: "
        for r in registration_parameters:
            print r.get_rotation(),r.get_shift()
        print "correct: "
        for r in correct_parameters:
            print r.get_rotation(),r.get_shift()
        for i in xrange(0,len(registration_parameters)):
            # Generate the registered projection
            imgx=em2d.Image()
            imgx.set_size(rows,cols)
            em2d.get_projection(imgx,particles,
                                      registration_parameters[i], options)
            ccc=em2d.get_cross_correlation_coefficient(subjects[i].get_data(),
                                                  imgx.get_data())
            print i,"ccc",ccc
            snr=0.5
            theoretical_ccc = (snr/(1.+snr))**.5
            self.assertAlmostEqual(ccc,theoretical_ccc, delta=0.02,
                 msg="Error in registration of subject %d: ccc %8.3f "\
                    "theoretical_ccc %8.3f "% (i,ccc,theoretical_ccc))
        os.remove(fn_registration_results)

if __name__ == '__main__':
    IMP.test.main()
