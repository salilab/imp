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
from math import *

class TestTools(IMP.test.TestCase):
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
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(mol, nonrigid_parts=mol.get_non_atomic_residues())
        rbs = dof.get_rigid_bodies()
        IMP.pmi.tools.shuffle_configuration(hier)

    def test_select_at_all_resolutions(self):
        """Test this actually gets everything"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        mol = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        atomic_res = mol.add_structure(self.get_input_file_name('chainA.pdb'),
                                       chain_id='A',
                                       res_range=(1,100))
        mol.add_representation(mol.get_atomic_residues(),resolutions=[0,10])
        mol.add_representation(mol.get_non_atomic_residues(), resolutions=[10])
        hier = s.build()

        ps = IMP.pmi.tools.select_at_all_resolutions(mol.get_hierarchy(),residue_index=93)
        self.assertEqual(len(ps),10) #should get res0 and res10

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
        testSystem = IMP.pmi.tools.input_adaptor(s,pmi_resolution=0)
        testState = IMP.pmi.tools.input_adaptor(st1,pmi_resolution=0)
        compareAll = [set(IMP.atom.Selection(m.get_hierarchy(),
                                         resolution=0).get_selected_particles()) for m in [m1,m2,m3]]

        self.assertEqual([set(l) for l in testSystem],compareAll)
        self.assertEqual([set(l) for l in testState],compareAll)

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

if __name__ == '__main__':
    IMP.test.main()
