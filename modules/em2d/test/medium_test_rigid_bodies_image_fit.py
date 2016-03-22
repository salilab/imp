from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.atom
import IMP.em2d
import IMP.algebra

class Tests(IMP.test.TestCase):

    def test_rigid_body_image_fit_restraint(self):
        """Test scoring with RigidBodiesImageFitRestraint"""
        m = IMP.Model()

        # read full complex
        fn = self.get_input_file_name("1z5s.pdb")
        prot = IMP.atom.read_pdb(fn, m, IMP.atom.ATOMPDBSelector())
        # read components
        names = ["1z5sA", "1z5sB", "1z5sC", "1z5sD"]
        fn_pdbs = [self.get_input_file_name(name + ".pdb") for name in names]
        components = [IMP.atom.read_pdb(fn, m, IMP.atom.ATOMPDBSelector())
                      for fn in fn_pdbs]
        components_rbs = [IMP.atom.create_rigid_body(c) for c in components]

        # img
        R = IMP.algebra.get_identity_rotation_3d()
        reg = IMP.em2d.RegistrationResult(R)
        img = IMP.em2d.Image()
        img.set_size(80, 80)

        resolution = 40
        pixel_size = 1.5
        options = IMP.em2d.ProjectingOptions(pixel_size, resolution)
        ls = IMP.core.get_leaves(prot)
        IMP.em2d.get_projection(img, ls, reg, options)
        # set restraint
        score_function = IMP.em2d.EM2DScore()
        rb_fit = IMP.em2d.RigidBodiesImageFitRestraint(score_function,
                                                       components_rbs, img)
        pp = IMP.em2d.ProjectingParameters(pixel_size, resolution)
        rb_fit.set_projecting_parameters(pp)
        # set the trivial case:
        n_masks = 1

        for rb in components_rbs:
            # set as the only possible orientation the one that the rigid
            # body already has
            rb_fit.set_orientations(rb,
                                    [rb.get_reference_frame().get_transformation_to().get_rotation()])
            self.assertEqual(rb_fit.get_number_of_masks(rb), n_masks,
                             "Incorrect number rigid body masks")

        # Calculate the positions of the rigid bodies respect to the centroid
        # of the entire molecule
        ls = IMP.core.get_leaves(prot)
        xyzs = IMP.core.XYZs(ls)
        centroid = IMP.core.get_centroid(xyzs)

        coords = [rb.get_coordinates() - centroid for rb in components_rbs]
        for rb, coord in zip(components_rbs, coords):
            rb.set_coordinates(coord)

        # Check that the value is a perfect registration
        score = rb_fit.evaluate(False)
        # It seems that projecting with the masks is slightly less accurate
        # I have to establish a tolerance of 0.03
        self.assertAlmostEqual(score, 0, delta=0.03)


if __name__ == '__main__':
    IMP.test.main()
