import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.em
import os
from math import *




#- test parametros de registro
#- test solucion con Fine2DREgistrationRestraint


class ProjectTests(IMP.test.TestCase):

    def test_registration(self):
        """Test the registration of 3 subjects from 1gyt.pdb at 0.5 SNR"""
#        self.assert_(True)

        # Get model from PDB file
        smodel = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        prot =  IMP.atom.read_pdb(self.get_input_file_name("1gyt.pdb"),
                                                              smodel,ssel)
        IMP.atom.add_radii(prot)
        particles=IMP.core.get_leaves(prot)
        # Read subject images
        srw = IMP.em.SpiderImageReaderWriter()
        selection_file=self.get_input_file_name("1gyt-subjects-0.5-SNR.sel")
        images_to_read_names=IMP.em2d.read_selection_file(selection_file)
        for i in xrange(0,len(images_to_read_names)):
            images_to_read_names[i] = self.get_input_file_name(
                                                  images_to_read_names[i])
        subjects = IMP.em.read_images(images_to_read_names,srw)
        self.assertEqual(len(subjects),3,"Problem reading subject images")

        # Generate 20 evenly distributed projections from the PDB file
        proj_params=IMP.em2d.evenly_distributed_registration_results(20)
        rows=128
        cols=128
        pixel_size = 1.5
        resolution = 1 # for generating projections, use a very high resolution
        interpolation_method=0;
        projections = IMP.em2d.generate_projections(particles,proj_params,
                    rows,cols,resolution,pixel_size,srw)
        self.assertEqual(len(projections),20,"Problem generating projections")
        # ====================================>
        # projnames=[]
        # for i in range(0,len(projections)):
        #    projnames.append(
        #              self.get_input_file_name("nonsense-"+str(i)+".spi"))
        # IMP.em.save_images(projections,projnames,srw)
        # <====================================
        # Prepare registration
        save_match_images=True
        coarse_registration_method = 1
        finder = IMP.em2d.ProjectionFinder();
        finder.initialize(pixel_size,resolution,coarse_registration_method,
                          save_match_images,interpolation_method);
        finder.set_model_particles(particles);
        finder.set_subjects(subjects);
        finder.set_projections(projections);
        finder.get_complete_registration();
        # Recover the registration results:
        registration_parameters=finder.get_registration_results()
        fn_registration_results=self.get_input_file_name(
                                                "my_1gyt_registration.params")
        IMP.em2d.write_registration_results(fn_registration_results,
                                          registration_parameters)
        # Read the correct registration results:
        correct_parameters=IMP.em2d.read_registration_results(
        self.get_input_file_name("1gyt-subjects-0.5-SNR.params") )
        for i in xrange(0,len(registration_parameters)):
            angle = IMP.em2d.rotation_error(registration_parameters[i],
                                            correct_parameters[i])
            dist = IMP.em2d.shift_error(registration_parameters[i],
                                        correct_parameters[i])
            angle_tolerance=0.05
            distance_tolerance = 1
            self.assertAlmostEqual(angle,0, delta=angle_tolerance,
                 msg="Rotation error %8.3f higher than tolerance %8.3f "
                     "for subject %d " % (angle,angle_tolerance,i))
            self.assertAlmostEqual(dist,0, delta=distance_tolerance,
                 msg="Translation error %8.3f higher than tolerance %8.3f "
                     "for subject %d " % (dist,distance_tolerance,i))
        # os.remove(fn_registration_results)

if __name__ == '__main__':
    IMP.test.main()
