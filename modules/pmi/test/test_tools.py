from __future__ import print_function
import IMP
import os
import IMP.test
import IMP.core
import IMP.container
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.io
import IMP.pmi.io.crosslink
import IMP.pmi.representation
import IMP.pmi.restraints
import IMP.pmi.restraints.em
import IMP.pmi.restraints.crosslinking
import IMP.pmi.macros
import RMF
import IMP.rmf
from math import *

class Tests(IMP.test.TestCase):
    def test_shuffle(self):
        """Test moving rbs, fbs"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        mol = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        atomic_res = mol.add_structure(self.get_input_file_name('chainA.pdb'),
                                       chain_id='A',
                                       res_range=(1,100))
        mol.add_representation(mol.get_atomic_residues(),resolutions=[10])
        mol.add_representation(mol.get_non_atomic_residues(), resolutions=[10])

        mol2 = mol.create_clone('B')
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        mv,rb1 = dof.create_rigid_body(mol, nonrigid_parts=mol.get_non_atomic_residues())
        mv,rb2 = dof.create_rigid_body(mol2, nonrigid_parts=mol2.get_non_atomic_residues())
        results = IMP.pmi.tools.shuffle_configuration(hier)

        ps1 = IMP.get_indexes(IMP.core.get_leaves(mol.get_hierarchy()))
        ps2 = IMP.get_indexes(IMP.core.get_leaves(mol2.get_hierarchy()))

        self.assertEqual(len(results),16)
        self.assertEqual(results[0],[rb1,set(ps2)])
        self.assertEqual(results[1],[rb2,set(ps1)])
        for r in results[2:]:
            self.assertFalse(r[1])

    def test_select_at_all_resolutions(self):
        """Test this actually gets everything"""
        try:
            import sklearn
        except:
            self.skipTest("Require sklearn for this test")
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        mol = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        atomic_res = mol.add_structure(self.get_input_file_name('chainA.pdb'),
                                       chain_id='A',
                                       res_range=(1,100))
        mol.add_representation(mol.get_atomic_residues(),
                               resolutions=[0,10],
                               density_prefix='testselect',
                               density_voxel_size=0,
                               density_residues_per_component=10)

        mol.add_representation(mol.get_non_atomic_residues(),
                               resolutions=[10],
                               setup_particles_as_densities=True)
        hier = s.build()

        ps = IMP.pmi.tools.select_at_all_resolutions(mol.get_hierarchy(),residue_index=93)
        self.assertEqual(len(ps),14) #should get res0, res10, and ALL densities
        os.unlink('testselect.txt')

    def test_get_name(self):
        """Test pmi::get_molecule_name_and_copy()"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        m2 = m1.create_clone('B')
        hier = s.build()
        sel0 = IMP.atom.Selection(hier,resolution=1,copy_index=0).get_selected_particles()
        self.assertEqual(IMP.pmi.get_molecule_name_and_copy(sel0[0]),"Prot1.0")
        sel1 = IMP.atom.Selection(hier,resolution=1,copy_index=1).get_selected_particles()
        self.assertEqual(IMP.pmi.get_molecule_name_and_copy(sel1[0]),"Prot1.1")

    def test_input_adaptor(self):
        """Test that input adaptor correctly performs selection"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        m2 = st1.create_molecule("Prot2",sequence=seqs["Protein_2"])
        m3 = st1.create_molecule("Prot3",sequence=seqs["Protein_3"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,63),offset=-54)
        a2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(180,192),offset=-179)
        a3 = m3.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='G',res_range=(55,63),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[1])
        m2.add_representation(a2,resolutions=[0,1]) # m2 only has atoms
        m3.add_representation(a3,resolutions=[1,10])
        m3.add_representation(m3.get_non_atomic_residues(),resolutions=[1], setup_particles_as_densities=True)
        hier = s.build()

        densities = [r.get_hierarchy() for r in m3.get_non_atomic_residues()]

        #set up GMM particles
        gemt = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                                                 self.get_input_file_name('prot_gmm.txt'),
                                                 target_is_rigid_body=True)

        gmm_hier = gemt.get_density_as_hierarchy()
        test0 = IMP.pmi.tools.input_adaptor(gmm_hier)
        self.assertEqual(test0, [gmm_hier])

        # get one resolution
        test1 = IMP.pmi.tools.input_adaptor(m1,pmi_resolution=0)
        self.assertEqual(test1,[IMP.atom.Selection(m1.get_hierarchy(),
                                                   resolution=0).get_selected_particles()])

        # get all resolutions
        test1all = IMP.pmi.tools.input_adaptor(m1,pmi_resolution='all')
        compare1all = set(IMP.atom.Selection(m1.get_hierarchy(),
                                             resolution=0).get_selected_particles()+
                          IMP.atom.Selection(m1.get_hierarchy(),
                                             resolution=1).get_selected_particles())
        self.assertEqual(set(test1all[0]),compare1all)

        # list of set of TempResidue
        test3 = IMP.pmi.tools.input_adaptor([m1[0:3],m2[:],m3[0:1]],
                                            pmi_resolution=1)
        compare3 = [IMP.atom.Selection(m1.get_hierarchy(),
                                       residue_indexes=[1,2,3],
                                       resolution=1).get_selected_particles(),
                    IMP.atom.Selection(m2.get_hierarchy(),
                                       resolution=1).get_selected_particles(),
                    IMP.atom.Selection(m3.get_hierarchy(),
                                       residue_index=1,
                                       resolution=1).get_selected_particles()]
        self.assertEqual([set(l) for l in test3],[set(l) for l in compare3])

        # nothing changes to hierarchy
        tH = [IMP.atom.Hierarchy(IMP.Particle(mdl))]
        testH = IMP.pmi.tools.input_adaptor(tH)
        self.assertEqual(testH,tH)

        # check passing system,state
        testSystem = [set(l) for l in
                      IMP.pmi.tools.input_adaptor(s,pmi_resolution=0)]
        testState = [set(l) for l in
                     IMP.pmi.tools.input_adaptor(st1,pmi_resolution=0)]
        compareAll = [set(IMP.atom.Selection(m.get_hierarchy(),
                                         resolution=0).get_selected_particles()) for m in [m1,m2,m3]]

        # get_molecules() returns a dict, so the order of testSystem
        # and testState is not guaranteed
        self.assertEqualUnordered(testSystem, compareAll)
        self.assertEqualUnordered(testState, compareAll)

    def assertEqualUnordered(self, a, b):
        """Compare two unordered lists; i.e. each list must have the
           same elements, but possibly in a different order"""
        self.assertEqual(len(a), len(b))
        for i in a + b:
            self.assertIn(i, a)
            self.assertIn(i, b)

    def test_get_is_canonical(self):
        """Test get PMI2 structures are canonical"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        hier = s.build()
        sel0 = IMP.atom.Selection(hier,molecule="Prot1",resolution=0).get_selected_particles()
        sel1 = IMP.atom.Selection(hier,molecule="Prot1",resolution=1).get_selected_particles()
        for p in sel0+sel1:
            self.assertTrue(IMP.pmi.get_is_canonical(p))

    def test_set_coordinates_from_rmf(self):
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1.get_residues()-a1,resolutions=1)
        hier = s.build()

        sel = IMP.atom.Selection(hier,resolution=IMP.atom.ALL_RESOLUTIONS).get_selected_particles()
        orig_coords = [IMP.core.XYZ(p).get_coordinates() for p in sel]
        fname = self.get_tmp_file_name('test_set_coords.rmf3')
        rh = RMF.create_rmf_file(fname)
        IMP.rmf.add_hierarchy(rh, hier)
        IMP.rmf.save_frame(rh)
        del rh
        for p in sel:
            IMP.core.transform(IMP.core.XYZ(p),IMP.algebra.Transformation3D([10,10,10]))
        coords1 = [IMP.core.XYZ(p).get_coordinates() for p in sel]
        for c0,c1 in zip(orig_coords,coords1):
            self.assertNotEqual(IMP.algebra.get_distance(c0,c1),0.0)
        IMP.pmi.tools.set_coordinates_from_rmf(hier,fname,0)
        coords2 = [IMP.core.XYZ(p).get_coordinates() for p in sel]
        for c0,c2 in zip(orig_coords,coords2):
            self.assertAlmostEqual(IMP.algebra.get_distance(c0,c2),0.0)

if __name__ == '__main__':
    IMP.test.main()
