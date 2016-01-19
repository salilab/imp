import IMP
import IMP.core
import IMP.algebra
import IMP.test

import IMP.pmi.restraints.em
import IMP.pmi.representation

import os
import operator
import math

class GaussianEMRestraintRigidBody(IMP.test.TestCase):

    def setUp(self):
        self.m = IMP.Model()
        self.simo1 = IMP.pmi.representation.Representation(
            self.m, upperharmonic=True, disorderedlength=False)


    def test_GaussianEMRestraint_rigid_body(self):
        fname = self.get_input_file_name('2A73.pdb50.txt')
        target_ps = []
        IMP.isd.gmm_tools.decorate_gmm_from_text(
            fname,
            target_ps,
            self.m,
            radius_scale=3.0,
            mass_scale=1.0)
        gemh = IMP.pmi.restraints.em.GaussianEMRestraint(target_ps, fname,
                                                         target_mass_scale=1.0,
                                                         slope=0.000001,
                                                         target_radii_scale=3.0,
                                                         target_is_rigid_body=True)
        gemh.set_label("Mobile")

        gemh.add_target_density_to_hierarchy(self.simo1.prot)

        gemh.add_to_model()
        gemh.set_weight(100.0)

        before=gemh.rs.evaluate(False)

        rb = gemh.rb
        rbxyz = (rb.get_x(), rb.get_y(), rb.get_z())

        transformation = IMP.algebra.get_random_local_transformation(
            rbxyz,
            100,
            math.pi)

        IMP.core.transform(rb, transformation)

        after=gemh.rs.evaluate(False)
        self.assertTrue(after>before)

if __name__ == '__main__':
    IMP.test.main()
