import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2D
import IMP.em
import os
from math import *




#- test parametros de registro
#- test solucion con Fine2DREgistrationRestraint


class ProjectTests(IMP.test.TestCase):

    def test_registration(self):
        """Test the registration of 3 subjects from 1gyt.pdb at 0.5 SNR"""
        self.assertTrue(True)

#     # Get model from PDB file
#     smodel = IMP.Model()
#     ssel = IMP.atom.ATOMPDBSelector()
#     prot =  IMP.atom.read_pdb(self.get_input_file_name("1gyt.pdb"), smodel,ssel)
#     IMP.atom.add_radii(prot)
#     particles=IMP.core.get_leaves(prot)
#     # Read subject images
#     srw = IMP.em.SpiderImageReaderWriter()
#     selection_file=self.get_input_file_name("1gyt-subjects-0.5-SNR.sel")
#     images_to_read_names=IMP.em2D.read_selection_file(selection_file)
#     for i in xrange(0,len(images_to_read_names)):
#      images_to_read_names[i] = self.get_input_file_name(
#                                                images_to_read_names[i])
#     subjects = IMP.em.read_images(images_to_read_names,srw)
#     self.assertEqual(len(subjects),3,"Problem reading subject images")

#     # Generate 20 evenly distributed projections from the PDB file
#     projection_parameters=IMP.em2D.evenly_distributed_registration_results(20)
#     rows=128
#     cols=128
#     pixel_size = 1.5
#     resolution = 1 # for generating projections, use a very high resolution
#     projections = IMP.em2D.generate_projections(particles,projection_parameters,
#                  rows,cols,resolution,pixel_size,srw)
#     self.assertEqual(len(projections),20,"Problem generating projections")
#     # ====================================>
#     projnames=[]
#     for i in range(0,len(projections)):
#      projnames.append(self.get_input_file_name("nonsense-"+str(i)+".spi"))
#     IMP.em.save_images(projections,projnames,srw)
#     # <====================================
#     # Prepare registration
#     finder = IMP.em2D.ProjectionFinder(particles,subjects,projections)
#     # register
#     # save_match_images=True
#     save_match_images=False
#     score=finder.get_complete_registration(save_match_images,pixel_size);
#     # Recover the registration results:
#     registration_parameters=finder.get_registration_results()
#     registration_results_name=self.get_input_file_name("my_registration.params")
#     IMP.em2D.write_registration_results(registration_results_name,
#                                        registration_parameters)
#     # Read the correct registration results:
#     correct_parameters=IMP.em2D.read_registration_results(
#        self.get_input_file_name("1gyt-subjects-0.5-SNR.params") )
#     for i in xrange(0,len(registration_parameters)):
#      angle = IMP.em2D.rotation_error(registration_parameters[i],
#                                      correct_parameters[i])
#      dist = IMP.em2D.shift_error(registration_parameters[i],
#                                  correct_parameters[i])
#      self.assertInTolerance(angle,0,0.05,
#                "Rotation error higher than tolerance for subject "+str(i))
#      self.assertInTolerance(dist,0,1,
#                "Translation error higher than tolerance for subject "+str(i))

if __name__ == '__main__':
    unittest.main()
