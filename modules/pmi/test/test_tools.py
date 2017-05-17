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
    def test_particle_to_sample_filter(self):
        """Test ParticleToSampleFilter"""
        class MockRestraint(object):
            def __init__(self, sos):
                self.sos = sos
            def get_particles_to_sample(self):
                return self.sos
        r1 = MockRestraint({'Nuisances_Sigma': ('p0', 'p1'),
                            'Nuisances_Psi': ('p2', 'p3')})
        r2 = MockRestraint({'Nuisances_Sigma': ('p0', 'p4')})
        with IMP.allow_deprecated():
            p = IMP.pmi.tools.ParticleToSampleFilter([r1, r2])
        p.add_filter('Sigma')
        ps = p.get_particles_to_sample()
        self.assertEqual(list(ps.keys()), ['Nuisances_Sigma'])
        val = ps['Nuisances_Sigma']
        self.assertEqual(sorted(val), ['p0', 'p0', 'p1', 'p4'])

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
        results = IMP.pmi.tools.shuffle_configuration(hier)

        ps1 = IMP.get_indexes(IMP.core.get_leaves(mol.get_hierarchy()))
        ps2 = IMP.get_indexes(IMP.core.get_leaves(mol2.get_hierarchy()))

        self.assertEqual(len(results),16)
        self.assertEqual(results[0],[rb1,set(ps2)])
        self.assertEqual(results[1],[rb2,set(ps1)])
        for r in results[2:]:
            self.assertFalse(r[1])

        # test it works if you pass particles
        r2 = IMP.pmi.tools.shuffle_configuration(IMP.core.get_leaves(hier))
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
                                                      bounding_box=((100,100,100),(200,200,200)))

        rbs_trans_after={}
        fbs_position_after={}

        rbs,fbs = IMP.pmi.tools.get_rbs_and_beads([hier])

        for rb in rbs:
            coor_rb = IMP.core.XYZ(rb).get_coordinates()
            self.assertTrue(100.0 <coor_rb[0]< 200.0)
            self.assertTrue(100.0 <coor_rb[1]< 200.0)
            self.assertTrue(100.0 <coor_rb[2]< 200.0)

        for fb in fbs:
            if IMP.core.NonRigidMember.get_is_setup(fb):
                coor_fb=IMP.algebra.Vector3D([fb.get_value(IMP.FloatKey(4)),
                                               fb.get_value(IMP.FloatKey(5)),
                                               fb.get_value(IMP.FloatKey(6))])
                self.assertTrue(100.0 <coor_fb[0]< 200.0)
                self.assertTrue(100.0 <coor_fb[1]< 200.0)
                self.assertTrue(100.0 <coor_fb[2]< 200.0)

            else:
                coor_fb=IMP.core.XYZ(fb).get_coordinates()
                self.assertTrue(100.0 <coor_fb[0]< 200.0)
                self.assertTrue(100.0 <coor_fb[1]< 200.0)
                self.assertTrue(100.0 <coor_fb[2]< 200.0)


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
                fbs_position_before[fb]=IMP.algebra.Vector3D(
                    [fb.get_value(IMP.FloatKey(4)),
                    fb.get_value(IMP.FloatKey(5)),
                    fb.get_value(IMP.FloatKey(6))])
            else:
                fbs_position_before[fb]=IMP.core.XYZ(fb).get_coordinates()

        IMP.pmi.tools.shuffle_configuration(hier)

        for rb in rbs:
            rbs_trans_after[rb]=rb.get_reference_frame().get_transformation_to()

        for fb in fbs:
            if IMP.core.NonRigidMember.get_is_setup(fb):
                fbs_position_after[fb]=IMP.algebra.Vector3D(
                    [fb.get_value(IMP.FloatKey(4)),
                    fb.get_value(IMP.FloatKey(5)),
                    fb.get_value(IMP.FloatKey(6))])
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
