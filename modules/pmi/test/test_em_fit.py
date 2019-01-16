from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.proteomics
import IMP.pmi.restraints.crosslinking
import IMP.pmi.restraints.em
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.macros

class TestGaussianEMRestraint(IMP.test.TestCase):

    def test_gaussian_em(self):
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),chain_id='A',
                                      res_range=(55,63),offset=-54)
        fname = self.get_tmp_file_name('test_gmm')
        m1.add_representation(atomic_res,resolutions=[1,10],
                              density_residues_per_component=2,
                              density_voxel_size=3.0,
                              density_prefix=fname)

        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[10],
                              setup_particles_as_densities=True)
        hier = m1.build()
        densities = IMP.atom.Selection(hier,representation_type=IMP.atom.DENSITIES).get_selected_particles()
        self.assertEqual(len(densities),6)

        # make sure you can score
        gem = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                                                        target_fn=self.get_input_file_name('prot_gmm.txt'),
                                                        target_is_rigid_body=True)
        self.assertEqual(gem.get_restraint(), gem.rs)
        info = gem.gaussianEM_restraint.get_static_info()
        info.set_was_used(True)
        self.assertEqual(info.get_number_of_filename(), 1)
        self.assertEqual(info.get_filename_key(0), 'filename')

        gem.get_restraint_set().set_was_used(True)
        gem.add_to_model()
        mdl.update()
        init_em_score = gem.evaluate()
        output = gem.get_output()
        self.assertEqual(sorted(output.keys()),
             ['GaussianEMRestraint_None', 'GaussianEMRestraint_None_CCC',
              'GaussianEMRestraint_sigma_None', '_TotalScore'])

        # get rigid body object
        rb = gem.get_rigid_body()
        self.assertEqual(IMP.core.RigidBody, type(rb))
        self.assertEqual(len(rb.get_rigid_members()), 1)

        #test target transformations
        p = gem.target_ps[0]
        pos = IMP.core.XYZ(p).get_coordinates()

        self.assertAlmostEqual(pos[0], -6.50710525063, delta=1e-6)
        self.assertAlmostEqual(pos[1], -44.7706839578, delta=1e-6)
        self.assertAlmostEqual(pos[2], -70.33819299, delta=1e-6)

        gem.center_target_density_on_model()
        pos = IMP.core.XYZ(p).get_coordinates()
        self.assertNotAlmostEqual(pos[0], -6.50710525063, delta=1e-6)

        gem.center_target_density_on_origin()
        pos = IMP.core.XYZ(p).get_coordinates()
        self.assertAlmostEqual(pos[0], 0, delta=1e-6)
        self.assertAlmostEqual(pos[1], 0, delta=1e-6)
        self.assertAlmostEqual(pos[2], 0, delta=1e-6)

    def test_add_em_gmms_to_hierarchy(self):
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
        gem.get_restraint_set().set_was_used(True)
        gem.set_label("em_1")
        gem.add_to_model()
        gem.add_target_density_to_hierarchy(st1)

        # Add a second gmm, which should become a second chain
        gem2 = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                                                        target_fn=self.get_input_file_name('prot_gmm.txt'),
                                                        target_is_rigid_body=True)

        gem2.get_restraint_set().set_was_used(True)
        gem2.set_label("em_2")
        gem2.add_to_model()
        gem2.add_target_density_to_hierarchy(st1)

        # Test that a two child molecules were added to State
        self.assertEqual(len(st1.get_hierarchy().get_children()), 3)


if __name__ == '__main__':
    IMP.test.main()
