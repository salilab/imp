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
import IMP.pmi.restraints
import IMP.pmi.restraints.em
import IMP.pmi.restraints.crosslinking
import IMP.pmi.macros
import RMF
import IMP.rmf
import sys
from math import *

class MockCommunicator(object):
    def __init__(self, numproc, rank):
        self.size, self.rank = numproc, rank
        # Data in transit from rank x to rank y; key is (x,y), value is data
        self.sent_data = {}

    def Get_rank(self):
        return self.rank

    def Barrier(self):
        pass

    def send(self, data, dest, tag):
        k = (self.rank, dest)
        if k not in self.sent_data:
            self.sent_data[k] = []
        self.sent_data[k].append(data)

    def recv(self, source, tag):
        return self.sent_data[(source, self.rank)].pop(0)


class MockMPI(object):
    def __init__(self, numproc, rank):
        self.COMM_WORLD = MockCommunicator(numproc, rank)


class MockMPI4Py(object):
    def __init__(self, numproc, rank):
        self.MPI = MockMPI(numproc, rank)


class Tests(IMP.test.TestCase):

    def test_particle_to_sample_list(self):
        """Test ParticleToSampleList"""
        p = IMP.pmi.tools.ParticleToSampleList()
        self.assertEqual(p.label, 'None')
        self.assertRaises(TypeError, p.add_particle, 'P0', 'bad_type', 1, 'foo')

        p.add_particle('RB1', 'Rigid_Bodies', (1., 2.), 'myRB1')
        # Test bad rigid body transformation
        self.assertRaises(TypeError, p.add_particle,
                          'RB1', 'Rigid_Bodies', [1., 2.], 'myRB1')

        p.add_particle('S1', 'Surfaces', (1., 2., 3.), 'myS1')
        self.assertRaises(TypeError, p.add_particle,
                          'S1', 'Surfaces', [1., 2.], 'myS1')

        p.add_particle('F1', 'Floppy_Bodies', 1., 'myF1')
        self.assertRaises(TypeError, p.add_particle,
                          'F1', 'Floppy_Bodies', 'badtransform', 'myF1')

        self.assertEqual(p.get_particles_to_sample(),
                {'SurfacesParticleToSampleList_myS1_None':
                        (['S1'], (1.0, 2.0, 3.0)),
                 'Rigid_BodiesParticleToSampleList_myRB1_None':
                        (['RB1'], (1.0, 2.0)),
                 'Floppy_BodiesParticleToSampleList_myF1_None': (['F1'], 1.0)})

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
        results = IMP.pmi.tools.shuffle_configuration(hier,return_debug=True)

        ps1 = IMP.get_indexes(IMP.core.get_leaves(mol.get_hierarchy()))
        ps2 = IMP.get_indexes(IMP.core.get_leaves(mol2.get_hierarchy()))

        self.assertEqual(len(results),16)
        self.assertEqual(results[0],[rb1,set(ps2)])
        self.assertEqual(results[1],[rb2,set(ps1)])
        for r in results[2:]:
            self.assertFalse(r[1])

        # test it works if you pass particles
        r2 = IMP.pmi.tools.shuffle_configuration(IMP.core.get_leaves(hier),return_debug=True)
        self.assertEqual(len(r2),16)
        self.assertEqual(r2[0],[rb1,set(ps2)])
        self.assertEqual(r2[1],[rb2,set(ps1)])
        for r in r2[2:]:
            self.assertFalse(r[1])

    def test_shuffle_box(self):
        """Test shuffling rbs, fbs with bounding box"""
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

        mol3 = st1.create_molecule("GCP2_YEAST_BEADS",sequence=seqs["GCP2_YEAST"][:100],chain_id='C')
        mol3.add_representation(mol3.get_non_atomic_residues(), resolutions=[10])

        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        mv,rb1 = dof.create_rigid_body(mol, nonrigid_parts=mol.get_non_atomic_residues())
        mv,rb2 = dof.create_rigid_body(mol2, nonrigid_parts=mol2.get_non_atomic_residues())
        results = IMP.pmi.tools.shuffle_configuration(hier,
                                                      bounding_box=((1000,1000,1000),
                                                      (1200,1200,1200)),return_debug=True)

        rbs_trans_after={}
        fbs_position_after={}

        rbs,fbs = IMP.pmi.tools.get_rbs_and_beads([hier])

        mdl.update()
        for rb in rbs:
            coor_rb = IMP.core.XYZ(rb).get_coordinates()
            self.assertTrue(1000.0 <coor_rb[0]< 1200.0)
            self.assertTrue(1000.0 <coor_rb[1]< 1200.0)
            self.assertTrue(1000.0 <coor_rb[2]< 1200.0)

        for fb in fbs:
            coor_fb=IMP.core.XYZ(fb).get_coordinates()
            self.assertTrue(1000.0 <coor_fb[0]< 1200.0)
            self.assertTrue(1000.0 <coor_fb[1]< 1200.0)
            self.assertTrue(1000.0 <coor_fb[2]< 1200.0)

    def test_shuffle_box_gaussian_beads(self):
        """Test shuffling Gaussian-decorated beads with bounding box"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        mol = st1.create_molecule(
            "GCP2_YEAST", sequence=seqs["GCP2_YEAST"][:10], chain_id='A')
        mol.add_representation(mol.get_non_atomic_residues(), resolutions=[10],
                               setup_particles_as_densities=True)
        hier = s.build()

        IMP.pmi.tools.shuffle_configuration(
            hier, bounding_box=((1000, 1000, 1000), (1200, 1200, 1200)))

    def test_shuffle_one_rigid_body(self):
        """Test shuffle of a single rigid body"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "ME", "X")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_rigid_body(nup84)
        rbs, fbs = IMP.pmi.tools.get_rbs_and_beads(hier)
        self.assertEqual(len(rbs), 1)
        self.assertEqual(len(fbs), 0)
        old_t = rbs[0].get_reference_frame().get_transformation_from()
        IMP.pmi.tools.shuffle_configuration(
            hier, bounding_box=((1000, 1000, 1000), (1200, 1200, 1200)))
        new_t = rbs[0].get_reference_frame().get_transformation_from()
        # Rigid body should have been translated from near the origin to
        # somewhere in the bounding box
        diff_t = old_t.get_translation() - new_t.get_translation()
        self.assertGreater(diff_t.get_magnitude(), 10.0)

    def test_shuffle_deep(self):
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

        mol3 = st1.create_molecule("GCP2_YEAST_BEADS",sequence=seqs["GCP2_YEAST"][:100],chain_id='C')
        mol3.add_representation(mol3.get_non_atomic_residues(), resolutions=[10])

        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(mol, nonrigid_parts=mol.get_non_atomic_residues())
        dof.create_rigid_body(mol2, nonrigid_parts=mol2.get_non_atomic_residues())
        dof.create_flexible_beads(mol3.get_non_atomic_residues(),max_trans=1.0)

        rbs,fbs = IMP.pmi.tools.get_rbs_and_beads([hier])

        rbs_trans_before={}
        fbs_position_before={}

        rbs_trans_after={}
        fbs_position_after={}


        for rb in rbs:
            rbs_trans_before[rb]=rb.get_reference_frame().get_transformation_to()

        for fb in fbs:
            if IMP.core.NonRigidMember.get_is_setup(fb):
                fbs_position_before[fb] = \
                        IMP.core.NonRigidMember(fb).get_internal_coordinates()
            else:
                fbs_position_before[fb]=IMP.core.XYZ(fb).get_coordinates()

        IMP.pmi.tools.shuffle_configuration(hier)

        for rb in rbs:
            rbs_trans_after[rb]=rb.get_reference_frame().get_transformation_to()

        for fb in fbs:
            if IMP.core.NonRigidMember.get_is_setup(fb):
                fbs_position_after[fb] = \
                        IMP.core.NonRigidMember(fb).get_internal_coordinates()
            else:
                fbs_position_after[fb]=IMP.core.XYZ(fb).get_coordinates()

        for fb in fbs:
            position_after=fbs_position_after[fb]
            position_before=fbs_position_before[fb]
            for i in [0,1,2]:
                self.assertNotEqual(position_after[i],position_before[i])

        for rb in rbs:
            position_after=rbs_trans_after[rb].get_translation()
            position_before=rbs_trans_before[rb].get_translation()
            rotation_after=rbs_trans_after[rb].get_rotation()*IMP.algebra.Vector3D(1,1,1)
            rotation_before=rbs_trans_before[rb].get_rotation()*IMP.algebra.Vector3D(1,1,1)
            for i in [0,1,2]:
                self.assertNotEqual(position_after[i],position_before[i])
                self.assertNotEqual(rotation_after[i],rotation_before[i])




    def test_select_all_resolutions_and_densities_sklearn(self):
        """Test this actually check selec_all_resoltions and get_densities"""
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
        rs=[0,10]
        mol.add_representation(mol.get_atomic_residues(),
                               resolutions=rs,
                               density_prefix='testselect',
                               density_voxel_size=0,
                               density_residues_per_component=10)

        mol.add_representation(mol.get_non_atomic_residues(),
                               resolutions=[10],
                               setup_particles_as_densities=True)
        hier = s.build()

        ps = IMP.pmi.tools.select_at_all_resolutions(mol.get_hierarchy(),residue_index=93)
        self.assertEqual(len(ps),14) #should get res0, res10, and ALL densities

        leaves=[]
        for r in rs:
            leaves+=IMP.atom.Selection(hier,resolution=r).get_selected_particles()

        dens=IMP.atom.Selection(hier,representation_type=IMP.atom.DENSITIES).get_selected_particles()
        leaves+=dens
        ps = IMP.pmi.tools.select_at_all_resolutions(mol.get_hierarchy())
        inds1=sorted(list(set([p.get_index() for p in leaves])))
        inds2=sorted(p.get_index() for p in ps)
        self.assertEqual(inds1,inds2)

        #check densities
        dens_test=IMP.pmi.tools.get_densities(hier)
        inds1=sorted(p.get_index() for p in dens)
        inds2=sorted(p.get_index() for p in dens_test)
        self.assertEqual(inds1,inds2)

        dens_test=IMP.pmi.tools.get_densities([mol])
        inds1=sorted(p.get_index() for p in dens)
        inds2=sorted(p.get_index() for p in dens_test)
        self.assertEqual(inds1,inds2)

        os.unlink('testselect.txt')

    def test_get_molecules(self):
        """Test that get_molecules correctly selected IMP.atom.Molecules"""
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
        ind1=m1.hier.get_particle_index()
        ind2=m2.hier.get_particle_index()
        ind3=m3.hier.get_particle_index()

        mols=IMP.pmi.tools.get_molecules(hier)
        inds=sorted(p.get_particle_index() for p in mols)
        self.assertEqual(inds,[ind1,ind2,ind3])

        mols=IMP.pmi.tools.get_molecules([m1,m2,m3])
        inds=sorted(p.get_particle_index() for p in mols)
        self.assertEqual(inds,[ind1,ind2,ind3])

        mols=IMP.pmi.tools.get_molecules([m1,m2])
        inds=sorted(p.get_particle_index() for p in mols)
        self.assertEqual(inds,[ind1,ind2])

        mols=IMP.pmi.tools.get_molecules(IMP.atom.get_leaves(m1.hier))
        inds=sorted(p.get_particle_index() for p in mols)
        self.assertEqual(inds,[ind1])

        mols=IMP.pmi.tools.get_molecules(IMP.atom.get_leaves(hier))
        inds=sorted(p.get_particle_index() for p in mols)
        self.assertEqual(inds,[ind1,ind2,ind3])

        mols=IMP.pmi.tools.get_molecules([IMP.atom.get_leaves(m1.hier)[0],IMP.atom.get_leaves(m3.hier)[1]])
        inds=sorted(p.get_particle_index() for p in mols)
        self.assertEqual(inds,[ind1,ind3])

    def test_select_at_all_resolutions_no_density(self):
        """More stringent and runs without sklearn"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        mol = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        rs=[0,1,3,10]
        atomic_res = mol.add_structure(self.get_input_file_name('chainA.pdb'),
                                       chain_id='A',
                                       res_range=(1,100))
        mol.add_representation(mol.get_atomic_residues(),
                               resolutions=rs,
                               density_residues_per_component=0)

        mol.add_representation(mol.get_non_atomic_residues(),
                               resolutions=[10],
                               setup_particles_as_densities=True)
        hier = s.build()
        leaves=[]
        for r in rs:
            leaves+=IMP.atom.Selection(hier,resolution=r).get_selected_particles()

        ps = IMP.pmi.tools.select_at_all_resolutions(mol.get_hierarchy())
        inds1=sorted(list(set([p.get_index() for p in leaves])))
        inds2=sorted(p.get_index() for p in ps)
        self.assertEqual(inds1,inds2)


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

    def test_get_densities(self):

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        st1 = s.create_state()
        m3 = st1.create_molecule("Prot3",sequence=seqs["Protein_3"])
        a3 = m3.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='G',res_range=(55,63),offset=-54)
        m3.add_representation(a3,resolutions=[1,10])
        m3.add_representation(m3.get_non_atomic_residues(),resolutions=[1], setup_particles_as_densities=True)
        hier = s.build()
        densities = IMP.pmi.tools.get_densities(m3)
        densities_test=IMP.atom.Selection(hier,representation_type=IMP.atom.DENSITIES).get_selected_particles()
        self.assertEqual(densities,densities_test)
        densities = IMP.pmi.tools.get_densities(m3.get_hierarchy())
        self.assertEqual(densities,densities_test)
        densities = IMP.pmi.tools.get_densities(hier)
        self.assertEqual(densities,densities_test)
        densities = IMP.pmi.tools.get_densities(IMP.atom.get_leaves(hier))
        self.assertEqual(densities,densities_test)

    def test_get_sorted_segments(self):
        """Test get_sorted_segments()"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1", sequence='ACGHAC')
        m1.add_representation(m1[:],resolutions=[1])
        m2 = st1.create_molecule("Prot2", sequence='ACGHAC')
        m2.add_representation(m2[:],resolutions=[10])
        hier = s.build()
        self.assertRaises(ValueError, IMP.pmi.tools.get_sorted_segments,
                          [m1[1:2], m2[:]])
        s = IMP.pmi.tools.get_sorted_segments([m1[1:2], m1[3:5]])
        s = [(x[0].get_name(), x[1].get_name(), x[2]) for x in s]
        self.assertEqual(s, [('2_bead', '2_bead', 2),
                             ('4_bead', '4_bead', 4),
                             ('5_bead', '5_bead', 5)])

    def test_input_adaptor_warn_slices(self):
        """Test that input adaptor warn_about_slices behavior"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1", sequence='ACGH')
        m1.add_representation(m1[:],resolutions=[10])
        hier = s.build()
        t = IMP.pmi.tools.input_adaptor(m1[0:2], pmi_resolution=1)

    def test_input_adaptor_pmi(self):
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
        self.assertEqual(test0, [IMP.atom.get_leaves(gmm_hier)])

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

        # check robustness and consistency TempResidue + Hierarchy
        test4 = IMP.pmi.tools.input_adaptor([m1[0:3],m2[:],m3[0:1]],flatten=True,pmi_resolution=1)

        compare4=IMP.pmi.tools.input_adaptor(compare3,pmi_resolution=1,flatten=True)
        compare5=IMP.pmi.tools.input_adaptor(compare3,flatten=True)
        compare6=IMP.pmi.tools.input_adaptor(compare3,pmi_resolution='all',flatten=True)

        self.assertEqual(test4,compare4)
        self.assertEqual(test4,compare5)
        self.assertEqual(test4,compare6)

        # check input is list or list of lists
        test5 = IMP.pmi.tools.input_adaptor([m1,m2,m3],
                                            pmi_resolution=1)
        test6 = IMP.pmi.tools.input_adaptor([[m1,m2],[m3]],
                                            pmi_resolution=1)
        self.assertEqual(test5,test6)

        # test input particles and input hierarchies

        compare7 = [IMP.atom.Hierarchy(p) for sublist in compare3 for p in sublist ]
        test7 = IMP.pmi.tools.input_adaptor(compare7,flatten=True,pmi_resolution=1)
        compare8 = [p for sublist in compare3 for p in sublist]
        test8 = IMP.pmi.tools.input_adaptor(compare8,flatten=True,pmi_resolution=1)
        self.assertEqual(test7,test8)

        # raises if passing an uneven list
        with self.assertRaises(Exception):
            IMP.pmi.tools.input_adaptor([[m1,m2],m3],pmi_resolution=1)

        #raises if passing mixed lists
        with self.assertRaises(Exception):
            IMP.pmi.tools.input_adaptor([m1,s,m3],pmi_resolution=1)

        # compare hierarchies and pmi molecules:
        m1s_pmi = IMP.pmi.tools.input_adaptor(m1,pmi_resolution=1)
        m1s_hiers = IMP.pmi.tools.input_adaptor(m1.hier,pmi_resolution=1)
        self.assertEqual([set(l) for l in m1s_pmi],[set(l) for l in m1s_hiers])

        m1s_pmi = IMP.pmi.tools.input_adaptor(m1,pmi_resolution=10)
        m1s_hiers = IMP.pmi.tools.input_adaptor(m1.hier,pmi_resolution=10)
        self.assertEqual([set(l) for l in m1s_pmi],[set(l) for l in m1s_hiers])

        m1s_pmi = IMP.pmi.tools.input_adaptor(m1,pmi_resolution='all')
        m1s_hiers = IMP.pmi.tools.input_adaptor(m1.hier,pmi_resolution='all')
        self.assertEqual([set(l) for l in m1s_pmi],[set(l) for l in m1s_hiers])

        # compare hierarchies and pmi states:
        st1s_pmi = IMP.pmi.tools.input_adaptor(st1,pmi_resolution=1,flatten=True)
        st1s_hiers = IMP.pmi.tools.input_adaptor(st1.hier,pmi_resolution=1,flatten=True)
        self.assertEqual(st1s_pmi,st1s_hiers)

        st1s_pmi = IMP.pmi.tools.input_adaptor(st1,pmi_resolution=10,flatten=True)
        st1s_hiers = IMP.pmi.tools.input_adaptor(st1.hier,pmi_resolution=10,flatten=True)
        self.assertEqual(st1s_pmi,st1s_hiers)

        st1s_pmi = IMP.pmi.tools.input_adaptor(st1,pmi_resolution='all',flatten=True)
        st1s_hiers = IMP.pmi.tools.input_adaptor(st1.hier,pmi_resolution='all',flatten=True)
        self.assertEqual(st1s_pmi,st1s_hiers)

        # compare hierarchies and pmi system:
        sys_pmi = IMP.pmi.tools.input_adaptor(s,pmi_resolution=1,flatten=True)
        sys_hiers = IMP.pmi.tools.input_adaptor(s.hier,pmi_resolution=1,flatten=True)
        self.assertEqual(sys_pmi,sys_hiers)

        sys_pmi = IMP.pmi.tools.input_adaptor(s,pmi_resolution=10,flatten=True)
        sys_hiers = IMP.pmi.tools.input_adaptor(s.hier,pmi_resolution=10,flatten=True)
        self.assertEqual(sys_pmi,sys_hiers)

        sys_pmi = IMP.pmi.tools.input_adaptor(s,pmi_resolution='all',flatten=True)
        sys_hiers = IMP.pmi.tools.input_adaptor(s.hier,pmi_resolution='all',flatten=True)
        self.assertEqual(sys_pmi,sys_hiers)

        # nothing changes to hierarchy
        p=IMP.Particle(mdl)
        h=IMP.atom.Hierarchy.setup_particle(p)
        IMP.atom.Mass.setup_particle(p,1.0)
        xyzr=IMP.core.XYZR.setup_particle(p)
        xyzr.set_coordinates((0,0,0))
        xyzr.set_radius(1.0)
        tH = [h]

        testH = IMP.pmi.tools.input_adaptor(tH)
        self.assertEqual(testH,[tH])

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

    def test_input_adaptor_non_pmi(self):
        mdl = IMP.Model()
        root=IMP.atom.Hierarchy(IMP.Particle(mdl))
        for i in range(10):
            p=IMP.Particle(mdl)
            h=IMP.atom.Hierarchy.setup_particle(p)
            IMP.atom.Mass.setup_particle(p,1.0)
            xyzr=IMP.core.XYZR.setup_particle(p)
            xyzr.set_coordinates((0,0,0))
            xyzr.set_radius(1.0)
            root.add_child(h)
        hs=IMP.pmi.tools.input_adaptor(root)
        self.assertEqual([IMP.atom.get_leaves(root)],hs)
        hs=IMP.pmi.tools.input_adaptor(root,pmi_resolution=1)
        self.assertEqual([IMP.atom.get_leaves(root)],hs)

    def test_Segments(self):
        self.assertRaises(TypeError, IMP.pmi.tools.Segments, 42.0)
        s=IMP.pmi.tools.Segments(1)
        self.assertEqual(s.segs,[[1]])
        s=IMP.pmi.tools.Segments([1])
        self.assertEqual(s.segs,[[1]])
        s=IMP.pmi.tools.Segments([1,1])
        self.assertEqual(s.segs,[[1]])
        s=IMP.pmi.tools.Segments([1,2])
        self.assertEqual(s.segs,[[1,2]])
        s=IMP.pmi.tools.Segments([1,2,3])
        self.assertEqual(s.segs,[[1,2,3]])
        s=IMP.pmi.tools.Segments([1,2,3,5])
        self.assertEqual(s.segs,[[1,2,3],[5]])
        s.add(6)
        self.assertRaises(TypeError, s.add, 42.0)
        self.assertEqual(s.segs,[[1,2,3],[5,6]])
        s.add(0)
        self.assertEqual(s.segs,[[0,1,2,3],[5,6]])
        s.add(3)
        self.assertEqual(s.segs,[[0,1,2,3],[5,6]])
        s.add(4)
        self.assertEqual(s.segs,[[0,1,2,3,4,5,6]])
        s.add([-3,-4])
        self.assertEqual(s.segs,[[-4,-3],[0,1,2,3,4,5,6]])
        s.remove(2)
        self.assertEqual(s.segs,[[-4,-3],[0,1],[3,4,5,6]])
        s.remove(5)
        self.assertEqual(s.segs,[[-4,-3],[0,1],[3,4],[6]])
        s.remove(5)
        self.assertEqual(s.segs,[[-4,-3],[0,1],[3,4],[6]])
        s.add(-1)
        self.assertEqual(s.segs,[[-4,-3],[-1,0,1],[3,4],[6]])
        s.remove(-4)
        self.assertEqual(s.segs,[[-3],[-1,0,1],[3,4],[6]])
        s.remove(1)
        self.assertEqual(s.segs,[[-3],[-1,0],[3,4],[6]])
        s.remove(6)
        self.assertEqual(s.segs,[[-3],[-1,0],[3,4]])
        self.assertEqual(s.get_flatten(), [-3,-1,0,3,4])
        self.assertEqual(repr(s), '[-3--3,-1-0,3-4]')

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

    def test_get_restraint_set(self):
        """Test get_restraint_set()"""
        m = IMP.Model()
        # Should make an empty set
        rs = IMP.pmi.tools.get_restraint_set(m)
        self.assertEqual(rs.get_number_of_restraints(), 0)

        for rmf in (True, False):
            rs = IMP.pmi.tools.get_restraint_set(m, rmf)
            self.assertEqual(rs.get_number_of_restraints(), 0)

    def test_add_restraint(self):
        """Test add_restraint_to_model()"""
        m = IMP.Model()

        r1 = IMP._ConstRestraint(m, [], 1)
        IMP.pmi.tools.add_restraint_to_model(m, r1, add_to_rmf=False)
        r2 = IMP._ConstRestraint(m, [], 1)
        IMP.pmi.tools.add_restraint_to_model(m, r2, add_to_rmf=True)

        rs = IMP.pmi.tools.get_restraint_set(m, rmf=False)
        self.assertEqual(rs.get_number_of_restraints(), 2)

        rs = IMP.pmi.tools.get_restraint_set(m, rmf=True)
        self.assertEqual(rs.get_number_of_restraints(), 1)

    def test_scatter_and_gather_no_mpi_list_rank_0(self):
        """Test scatter_and_gather of lists without using MPI, rank==0"""
        mpi4py = MockMPI4Py(3, 0)
        sys.modules['mpi4py'] = mpi4py
        try:
            # Simulate data from the other two processes
            mpi4py.MPI.COMM_WORLD.sent_data[1, 0] = [[1,2]]
            mpi4py.MPI.COMM_WORLD.sent_data[2, 0] = [[3,4]]
            data = IMP.pmi.tools.scatter_and_gather([9,10])
            self.assertEqual(data, [9,10,1,2,3,4])
            self.assertEqual(mpi4py.MPI.COMM_WORLD.sent_data[0, 1], [data])
            self.assertEqual(mpi4py.MPI.COMM_WORLD.sent_data[0, 2], [data])
        finally:
            del sys.modules['mpi4py']

    def test_scatter_and_gather_no_mpi_bad_type_rank_0(self):
        """Test scatter_and_gather of unsupported type without using MPI"""
        mpi4py = MockMPI4Py(3, 0)
        sys.modules['mpi4py'] = mpi4py
        try:
            # Simulate data from the other two processes
            mpi4py.MPI.COMM_WORLD.sent_data[1, 0] = [[1,2]]
            mpi4py.MPI.COMM_WORLD.sent_data[2, 0] = [[3,4]]
            self.assertRaises(TypeError, IMP.pmi.tools.scatter_and_gather, 42.0)
        finally:
            del sys.modules['mpi4py']

    def test_scatter_and_gather_no_mpi_dict_rank_0(self):
        """Test scatter_and_gather of dicts without using MPI, rank==0"""
        mpi4py = MockMPI4Py(3, 0)
        sys.modules['mpi4py'] = mpi4py
        try:
            # Simulate data from the other two processes
            mpi4py.MPI.COMM_WORLD.sent_data[1, 0] = [{'a':'b'}]
            mpi4py.MPI.COMM_WORLD.sent_data[2, 0] = [{'c':'d'}]
            data = IMP.pmi.tools.scatter_and_gather({'e':'f'})
            self.assertEqual(data, {'a':'b', 'c':'d', 'e':'f'})
            self.assertEqual(mpi4py.MPI.COMM_WORLD.sent_data[0, 1], [data])
            self.assertEqual(mpi4py.MPI.COMM_WORLD.sent_data[0, 2], [data])
        finally:
            del sys.modules['mpi4py']

    def test_scatter_and_gather_no_mpi_list_rank_1(self):
        """Test scatter_and_gather of lists without using MPI, rank!=0"""
        mpi4py = MockMPI4Py(3, 1)
        sys.modules['mpi4py'] = mpi4py
        try:
            # Simulate data from rank==0
            mpi4py.MPI.COMM_WORLD.sent_data[0, 1] = [[9,10,1,2,3,4]]
            data = IMP.pmi.tools.scatter_and_gather([1,2])
            self.assertEqual(data, [9,10,1,2,3,4])
            self.assertEqual(mpi4py.MPI.COMM_WORLD.sent_data[1, 0], [[1,2]])
        finally:
            del sys.modules['mpi4py']

    def test_scatter_and_gather_mpi(self):
        """Test scatter_and_gather using MPI"""
        try:
            from mpi4py import MPI
        except ImportError:
            self.skipTest("No MPI support")
        comm = MPI.COMM_WORLD
        numproc = comm.size
        rank = comm.Get_rank()
        data = IMP.pmi.tools.scatter_and_gather([rank])
        self.assertEqual(data, list(range(numproc)))
        data = IMP.pmi.tools.scatter_and_gather({rank:'x'})
        self.assertEqual(data, dict.fromkeys(range(numproc), 'x'))

    def test_sublist_iterator(self):
        """Test sublist_iterator()"""
        inp = ['a', 'b', 'c']
        out = IMP.pmi.tools.sublist_iterator(inp)
        self.assertEqual(list(out),
                         [['a'], ['a', 'b'], ['a', 'b', 'c'],
                          ['b'], ['b', 'c'], ['c']])

        out = IMP.pmi.tools.sublist_iterator(inp, lmin=2)
        self.assertEqual(list(out), [['a', 'b'], ['a', 'b', 'c'], ['b', 'c']])

        out = IMP.pmi.tools.sublist_iterator(inp, lmax=2)
        self.assertEqual(list(out),
                         [['a'], ['a', 'b'], ['b'], ['b', 'c'], ['c']])

    def test_flatten_list(self):
        """Test flatten_list()"""
        inp = [['a', 'b', 'c'], ['d', 'e']]
        self.assertEqual(IMP.pmi.tools.flatten_list(inp),
                         ['a', 'b', 'c', 'd', 'e'])

    def test_color_change(self):
        """Test ColorChange class"""
        cc = IMP.pmi.tools.ColorChange()
        self.assertEqual(cc.triplet((1,20,200), 'x'), '0114c8')
        self.assertEqual(cc.triplet((1,20,200)), '0114c8')
        self.assertEqual(cc.triplet((1,20,200), 'X'), '0114C8')
        self.assertEqual([int(x) for x in cc.rgb('0114C8')], [1,20,200])
        self.assertEqual([int(x) for x in cc.rgb('0114c8')], [1,20,200])

    def test_color2rgb(self):
        """Test color2rgb function"""
        r,g,b = IMP.pmi.tools.color2rgb("tan")
        self.assertAlmostEqual(r, 0.824, delta=0.01)
        self.assertAlmostEqual(g, 0.706, delta=0.01)
        self.assertAlmostEqual(b, 0.549, delta=0.01)

        self.assertRaises(KeyError, IMP.pmi.tools.color2rgb, "bad color")

        r,g,b = IMP.pmi.tools.color2rgb("#6495ed")
        self.assertAlmostEqual(r, 0.392, delta=0.01)
        self.assertAlmostEqual(g, 0.584, delta=0.01)
        self.assertAlmostEqual(b, 0.929, delta=0.01)

    def test_ordered_set(self):
        """Test OrderedSet class"""
        s = IMP.pmi.tools.OrderedSet((8,1,2,10,1,8))
        self.assertEqual(list(s), [8, 1, 2, 10])
        self.assertEqual(len(s), 4)
        self.assertIn(8, s)
        self.assertNotIn(80, s)
        s.add(4)
        self.assertIn(4, s)
        self.assertEqual(len(s), 5)
        self.assertEqual(list(s), [8, 1, 2, 10, 4])
        s.add(8)
        self.assertEqual(len(s), 5)
        s.discard(8)
        self.assertEqual(len(s), 4)
        s.discard(8)
        self.assertEqual(len(s), 4)
        self.assertEqual(list(reversed(s)), [4, 10, 2, 1])
        self.assertEqual(s, IMP.pmi.tools.OrderedSet((1,2,10,4)))
        self.assertNotEqual(s, IMP.pmi.tools.OrderedSet((1,2,4,10)))
        self.assertEqual(s.pop(False), 1)
        self.assertEqual(s.pop(True), 4)
        self.assertEqual(s.pop(), 10)
        self.assertEqual(s.pop(), 2)
        self.assertRaises(KeyError, s.pop)
        self.assertEqual(len(s), 0)

    def test_ordered_default_dict_none(self):
        """Test OrderedDefaultDict class with None"""
        self.assertRaises(TypeError, IMP.pmi.tools.OrderedDefaultDict, 42.0)
        d = IMP.pmi.tools.OrderedDefaultDict(None)
        self.assertNotIn('a', d)
        d['a'] = 'b'
        self.assertIn('a', d)
        self.assertEqual(d['a'], 'b')
        self.assertRaises(KeyError, lambda x: x['b'], d)
        d['6'] = None
        d['4'] = None
        d['3'] = None
        # Keys should be ordered
        self.assertEqual(list(d.keys()), ['a', '6', '4', '3'])

    def test_ordered_default_dict_list(self):
        """Test OrderedDefaultDict class with list"""
        d = IMP.pmi.tools.OrderedDefaultDict(list)
        self.assertNotIn('a', d)
        d['a'] = 'b'
        self.assertIn('a', d)
        self.assertEqual(d['a'], 'b')
        self.assertNotIn('b', d)
        # Test default value
        self.assertEqual(d['b'], [])
        cls, args, n1, n2, items = d.__reduce__()
        self.assertEqual(list(items), [('a', 'b'), ('b', [])])
        d['6'] = None
        d['4'] = None
        d['3'] = None
        # Keys should be ordered
        self.assertEqual(list(d.keys()), ['a', 'b', '6', '4', '3'])


if __name__ == '__main__':
    IMP.test.main()
