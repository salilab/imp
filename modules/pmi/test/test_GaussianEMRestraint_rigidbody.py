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
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        self.simo1 = IMP.pmi.representation.Representation(
            self.m, upperharmonic=True, disorderedlength=False)

    def test_GaussianEMRestraint_rigid_body(self):
        """Test rigid body movement of target EM map"""
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

    def test_add_em_gmms_to_state(self):
        """Test adding EM Restraint GMMs to PMI2 Hierarchies and RMF"""

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()

        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])


        m1.add_representation(m1.get_residues(),resolutions=[1], setup_particles_as_densities=True)
        hier = m1.build()
        densities = IMP.atom.Selection(hier,representation_type=IMP.atom.DENSITIES).get_selected_particles()

        gem = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                                                        target_fn=self.get_input_file_name('prot_gmm.txt'),
                                                        target_is_rigid_body=True)
        gem.set_label("em_1")
        gem.add_to_model()
        gem.add_target_density_to_hierarchy(st1)

        # Add a second gmm, which should become a second chain, B
        gem2 = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                                                        target_fn=self.get_input_file_name('prot_gmm.txt'),
                                                        target_is_rigid_body=True)
        gem2.set_label("em_2")
        gem2.add_to_model()
        gem2.add_target_density_to_hierarchy(st1)

        # Test that a two child molecules were added to State
        self.assertEqual(len(st1.get_hierarchy().get_children()), 3)
if __name__ == '__main__':
    IMP.test.main()
