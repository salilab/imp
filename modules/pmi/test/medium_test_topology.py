from __future__ import print_function, division
import IMP
import IMP.atom
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.tools
import IMP.test
import RMF
import IMP.rmf
import IMP.pmi.dof
import IMP.pmi.macros
import os


def get_atomic_residue_list(residues):
    r1 = []
    for r in residues:
        ps = r.get_hierarchy().get_children()
        if len(ps) == 0:
            r1.append('-')
        else:
            r1.append(IMP.atom.get_one_letter_code(r.get_residue_type()))
    return ''.join(r1)


class MultiscaleTopologyTest(IMP.test.TestCase):

    def initialize_system(self, mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()

        # Read sequences and create Molecules
        seqs = IMP.pmi.topology.Sequences(
            IMP.pmi.get_example_path('data/gcp2.fasta'))
        mol = st1.create_molecule(
            "GCP2", sequence=seqs["GCP2_YEAST"][:100], chain_id='A')

        # Add structure. This function returns a list of the residues that now
        # have structure
        a1 = mol.add_structure(IMP.pmi.get_example_path('data/gcp2.pdb'),
                               res_range=(1, 100),
                               chain_id='A')

        # Add representations. For structured regions, created a few beads as well as densities
        # For unstructured regions, create a single bead level and set those up
        # as densities
        gmm_prefix = self.get_input_file_name('gcp2_gmm.txt').strip('.txt')
        mol.add_representation(a1,
                               resolutions=[10, 100],
                               density_prefix=gmm_prefix,
                               density_residues_per_component=20,
                               density_voxel_size=3.0)
        mol.add_representation(mol.get_non_atomic_residues(),
                               resolutions=[10],
                               setup_particles_as_densities=True)

        # When you call build, this actually makes the beads and fits the GMMs
        #  This returns a canonical IMP hierarchy
        hier = s.build()

        return a1, hier, mol

    def test_molecule_set_behaviour2(self):
        '''
        Here we assert that residue sets are not changed before and after
        the model build. The current behaviour does not make the test pass.
        '''

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()

        # Read sequences and create Molecules
        seqs = IMP.pmi.topology.Sequences(
            IMP.pmi.get_example_path('data/gcp2.fasta'))
        mol = st1.create_molecule(
            "GCP2", sequence=seqs["GCP2_YEAST"][:120], chain_id='A')

        # Add structure. This function returns a list of the residues that now
        # have structure
        a1 = mol.add_structure(IMP.pmi.get_example_path('data/gcp2.pdb'),
                               res_range=(1, 100),
                               chain_id='A')

        # Add representations. For structured regions, created a few beads as well as densities
        # For unstructured regions, create a single bead level and set those up
        # as densities
        gmm_prefix = self.get_input_file_name('gcp2_gmm.txt').strip('.txt')
        mol.add_representation(a1,
                               resolutions=[10, 100],
                               density_prefix=gmm_prefix,
                               density_residues_per_component=20,
                               density_voxel_size=3.0)

        # before we add the beads representation we chack the consistency
        # of mol.represented with the list obtained by removing it from the
        # whole molecule, and store the list of results
        mol_beads_1 = mol[:] - mol.get_represented()
        #we will need this later
        mol_beads_2 = mol[:] - mol.get_represented()
        status_1 = []
        for x in mol:
            status_1.append(x in mol_beads_1)
            self.assertEqual(x in mol_beads_1, not x in mol.get_represented())

        mol.add_representation(mol_beads_1,
                               resolutions=[10],
                               setup_particles_as_densities=True)

        before1 = len(mol_beads_1)
        inter = mol[0:100] & mol_beads_1
        mol_beads_1 -= inter
        after1 = len(mol_beads_1)
        # these are the number of residues mapped to anty bead
        self.assertEqual(before1, 83)
        # these are thwe number of residues mapped in beads but no in the range
        # [1:100]
        self.assertEqual(after1, 20)

        hier = s.build()

        before2 = len(mol_beads_2)
        inter = mol[0:100] & mol_beads_2
        mol_beads_2 -= inter
        after2 = len(mol_beads_2)
        self.assertEqual(before2, before1)
        #we expect this to work
        self.assertEqual(after2, after1)

    def test_molecule_set_behaviour(self):
        '''
        Here we assert that residue sets are not changed before and after
        the model build. The current behaviour does not make the test pass.
        '''

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()

        # Read sequences and create Molecules
        seqs = IMP.pmi.topology.Sequences(
            IMP.pmi.get_example_path('data/gcp2.fasta'))
        mol = st1.create_molecule(
            "GCP2", sequence=seqs["GCP2_YEAST"][:100], chain_id='A')

        # Add structure. This function returns a list of the residues that now
        # have structure
        a1 = mol.add_structure(IMP.pmi.get_example_path('data/gcp2.pdb'),
                               res_range=(1, 100),
                               chain_id='A')

        # Add representations. For structured regions, created a few beads as well as densities
        # For unstructured regions, create a single bead level and set those up
        # as densities
        gmm_prefix = self.get_input_file_name('gcp2_gmm.txt').strip('.txt')
        mol.add_representation(a1,
                               resolutions=[10, 100],
                               density_prefix=gmm_prefix,
                               density_residues_per_component=20,
                               density_voxel_size=3.0)

        # before we add the beads representation we chack the consistency
        # of mol.represented with the list obtained by removing it from the
        # whole molecule, and store the list of results
        mol_beads_1 = mol[:] - mol.get_represented()
        status_1 = []
        for x in mol:
            status_1.append(x in mol_beads_1)
            self.assertEqual(x in mol_beads_1, not x in mol.get_represented())

        mol.add_representation(mol_beads_1,
                               resolutions=[10],
                               setup_particles_as_densities=True)

        # after adding the beads to the representation,
        # we check that the residues are all represented (ie, no residue in mol_beads_2)
        # and store the result

        mol_beads_2 = mol[:] - mol.get_represented()
        status_2 = []
        for x in mol:
            status_2.append(x in mol_beads_2)
            self.assertEqual(x in mol_beads_2, not x in mol.get_represented())
            self.assertFalse(x in mol_beads_2)

        hier = s.build()

        # after we build the coordinates, we shouldn't get any residue in mol_beads_3,
        # but it is not the case.

        mol_beads_3 = mol[:] - mol.get_represented()
        # when we print the results, there are big inconsistencies
        # with the expected behaviour. We expect that the sixth column is indentical to the fifth and the second;
        # and we expect that the first is equal to the first. But this is not obtained :-)
        # The results are not intuitive at this stage, we think that mol has changed after build,
        # and mol.get_represented() is not changed correspondigly.

        #for n, x in enumerate(mol):
        #    print(x, status_1[n], status_2[n], x in mol_beads_1,
        #          x in mol_beads_2, x in mol_beads_3)

        # and this test fails

        for x in mol:
            self.assertEqual(x in mol_beads_3, not x in mol.get_represented())
            self.assertFalse(x in mol_beads_3)

    def test_num_residues(self):
        """ Test different ways of accessing residues"""
        mdl = IMP.Model()
        (a1, hier, mol) = self.initialize_system(mdl)
        #print(a1, mol.get_atomic_residues())

        self.assertEqual(37, len(a1))                # now these are duplicated
        # only 5 after build()!
        self.assertEqual(len(a1), len(mol.get_atomic_residues()))
        self.assertEqual(len(mol.get_residues()),
                         len(mol.get_atomic_residues()) + len(mol.get_non_atomic_residues()))
        # now contains only beads that are built
        self.assertEqual(len(mol.get_residues()), 100)

    def test_num_unstruct_res(self):
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        (a1, hier, mol) = self.initialize_system(mdl)

        struct_res = 0
        unstruct_res = 0
        for res in mol.residues:
            if res.get_has_structure():
                struct_res += 1
            else:
                unstruct_res += 1

        self.assertEqual(63, unstruct_res)
        self.assertEqual(37, struct_res)
        self.assertEqual(len(mol.residues), unstruct_res + struct_res)

    def test_residue_print(self):
        """PMI Residues cannot print their name
        The self.hier object in TempResidue is an IMP.atom.Hierarchy
        IMP.atom.Hierarchy does not have a function get_residue_type()
        """
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        (a1, hier, mol) = self.initialize_system(mdl)
        res = mol.residues[0]
        try:
            print(res)
        except:
            self.fail("Cannot print Residue")
        try:
            print(res.get_code())
        except:
            self.fail("Cannot print residue code")

    def test_molecule_rigid_members(self):
        """None of the leaves of the molecule are RigidMembers"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        (a1, hier, mol) = self.initialize_system(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(mol,
                              nonrigid_parts=mol.get_non_atomic_residues())
        rb = IMP.core.RigidBodyMember(
            IMP.atom.get_leaves(mol.get_hierarchy())[0]).get_rigid_body()
        nrms = 0
        rms = 0
        all_things = IMP.atom.get_leaves(mol.get_hierarchy())
        for part in all_things:
            if IMP.core.NonRigidMember.get_is_setup(part):
                nrms += 1
            elif IMP.core.RigidMember.get_is_setup(part):
                rms += 1
            else:
                self.fail("Particle not a RigidMember or a NonRigidMember")
        self.assertNotEqual(0, rms)

    def test_molecule_rigid_members1(self):
        """None of the leaves of the selection (Resolution=10) are RigidMembers"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        (a1, hier, mol) = self.initialize_system(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)

        # Create rigid body
        dof.create_rigid_body(mol,
                              nonrigid_parts=mol.get_non_atomic_residues())
        rb = IMP.core.RigidBodyMember(
            IMP.atom.get_leaves(mol.get_hierarchy())[0]).get_rigid_body()
        nrms = 0
        rms = 0

        selection = IMP.atom.Selection(
            hierarchy=mol.get_hierarchy(), resolution=10).get_hierarchies()
        all_things = IMP.atom.get_leaves(selection[0])
        for part in all_things:
            if IMP.core.NonRigidMember.get_is_setup(part):
                nrms += 1
            elif IMP.core.RigidMember.get_is_setup(part):
                rms += 1
            else:
                self.fail("Particle not a RigidMember or a NonRigidMember")
        self.assertNotEqual(0, rms)

    def test_molecule_rigid_members2(self):
        """When the rigid body is created with no assigned nonrigid members
        all leaves of the molecule are of type RigidMember
        """
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        (a1, hier, mol) = self.initialize_system(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)

        # Create rigid body
        dof.create_rigid_body(mol)
        rb = IMP.core.RigidBodyMember(
            IMP.atom.get_leaves(mol.get_hierarchy())[0]).get_rigid_body()
        nrms = 0
        rms = 0

        selection = IMP.atom.Selection(
            hierarchy=mol.get_hierarchy(), resolution=10).get_hierarchies()
        all_things = IMP.atom.get_leaves(selection[0])

        for part in all_things:
            if IMP.core.NonRigidMember.get_is_setup(part):
                nrms += 1
            elif IMP.core.RigidMember.get_is_setup(part):
                rms += 1
            else:
                self.fail("Particle not a RigidMember or a NonRigidMember")
        self.assertEqual(0, nrms)


class Tests(IMP.test.TestCase):

    def test_read_sequences(self):
        """Test if the sequence reader returns correct strings"""
        # test without name map
        seqs0 = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'))
        self.assertEqual(len(seqs0), 3)
        self.assertEqual(seqs0['Protein_1'], 'QEALVVKDLL')
        self.assertEqual(seqs0['Protein_2'], 'PEEDILKYVSYTL')
        self.assertEqual(seqs0['Protein_3'], 'QEALVVKDLL')

        # test with name map
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})
        self.assertEqual(len(seqs), 3)
        self.assertEqual(seqs['Prot1'], 'QEALVVKDLL')
        self.assertEqual(seqs['Prot2'], 'PEEDILKYVSYTL')
        self.assertEqual(seqs['Prot3'], 'QEALVVKDLL')

    def test_create_states(self):
        """Test State-creation from System"""
        s = IMP.pmi.topology.System()
        for i in range(10):
            self.assertEqual(s.get_number_of_states(), i)
            st = s.create_state()
            self.assertEqual(st.get_hierarchy().get_parent(),
                             s.get_hierarchy())
            self.assertEqual(st.mdl, s.mdl)
        self.assertEqual(s.get_number_of_states(), 10)

    def test_create_molecules(self):
        """Test Molecule creation from State"""
        s = IMP.pmi.topology.System()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})

        # create state 1 with 2 molecules
        st = s.create_state()
        m1 = st.create_molecule("Prot1", sequence=seqs["Prot1"])
        m2 = st.create_molecule("Prot2", sequence=seqs["Prot2"])
        self.assertEqual(m1.get_hierarchy().get_parent(), st.get_hierarchy())
        self.assertEqual(m2.get_hierarchy().get_parent(), st.get_hierarchy())
        self.assertEqual(m1.mdl, st.mdl)
        self.assertEqual(m2.mdl, st.mdl)
        self.assertEqual(m1.get_name(), "Prot1")
        self.assertEqual(m2.get_name(), "Prot2")
        self.assertEqual(len(st.get_hierarchy().get_children()), 2)

        # create state 2 with one molecule
        st2 = s.create_state()
        m3 = st2.create_molecule("Prot3", sequence=seqs["Prot3"])
        self.assertEqual(m3.get_hierarchy().get_parent(), st2.get_hierarchy())
        self.assertEqual(m3.mdl, st2.mdl)
        self.assertEqual(m3.get_name(), "Prot3")
        self.assertEqual(len(st2.get_hierarchy().get_children()), 1)

        # test if sequences are OK
        self.assertEqual(''.join(r.get_code()
                         for r in m1.residues), seqs["Prot1"])
        self.assertEqual(''.join(r.get_code()
                         for r in m2.residues), seqs["Prot2"])
        self.assertEqual(''.join(r.get_code()
                         for r in m3.residues), seqs["Prot3"])

    def test_add_structure(self):
        """Test adding partial structure data to a molecule"""

        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})

        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"])
        m2 = st1.create_molecule("Prot2", sequence=seqs["Prot2"])
        res1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                chain_id='A', res_range=(55, 63), offset=-54)
        res2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                                chain_id='B', res_range=(180, 192), offset=-179)

        # check that the molecule residues have the right info
        rlist1 = get_atomic_residue_list(m1.residues)
        rlist2 = get_atomic_residue_list(m2.residues)
        self.assertEqual(rlist1, 'QE--VVKDL-')
        self.assertEqual(rlist2, 'PEEDILKYVSYTL')

        # check that the returned Residue index sets are correct
        self.assertEqual(res1, set([m1.residues[i]
                         for i in (0, 1, 4, 5, 6, 7, 8)]))
        self.assertEqual(res2, set([m2.residues[i] for i in range(0, 13)]))

    def test_get_atomic_non_atomic_residues(self):
        """test if, adding a structure, you get the atomic and non atomic residues sets
        correctly"""
        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2'})

        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"])
        m2 = st1.create_molecule("Prot2", sequence=seqs["Prot2"])
        res1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                chain_id='A', res_range=(55, 63), offset=-54)

        m1_atomic_residues = m1.get_atomic_residues()
        m1_non_atomic_residues = m1.get_non_atomic_residues()
        m1_all_residues = m1.get_residues()
        m2_atomic_residues = m2.get_atomic_residues()
        m2_non_atomic_residues = m2.get_non_atomic_residues()
        m2_all_residues = m2.get_residues()

        self.assertEqual(m1_atomic_residues,
                         m1_all_residues - m1_non_atomic_residues)
        self.assertEqual(m2_atomic_residues,
                         m2_all_residues - m2_non_atomic_residues)

    def test_residue_access(self):
        """test functions to retrieve residues"""
        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})

        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"])
        self.assertEqual(m1[:], set(m1.residues[:]))
        self.assertEqual(m1[1], m1.residues[1])
        self.assertEqual(m1[1:5], set(m1.residues[1:5]))
        self.assertEqual(m1['1'], m1.residues[0])
        self.assertEqual(m1.residue_range(1, 5), set(m1.residues[1:6]))
        self.assertEqual(m1.residue_range('2', '6'), set(m1.residues[1:6]))
        inv = m1[:] - m1[1:5]
        self.assertEqual(inv, set([m1.residues[0]] + m1.residues[5:10]))

    def test_build_system(self):
        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1", sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(
            self.get_input_file_name('prot.pdb'), chain_id='A',
            res_range=(55, 63), offset=-54)
        non_atomic_res = m1.get_residues() - atomic_res

        m1.add_representation(atomic_res, resolutions=[0, 1, 10])
        m1.add_representation(non_atomic_res, resolutions=[10])
        hier = m1.build()
        frags = hier.get_children()

        # check names
        self.assertEquals(hier.get_name(), 'Prot1')
        self.assertEquals(m1.__repr__(), 'System.State_0.Prot1.0')

        # check if res0,1 created correctly
        for rnum, rname, anums in zip((1, 2, 5, 6, 7, 8, 9), 'QEVVKDL', (9, 9, 7, 7, 9, 8, 8)):
            res = IMP.atom.Selection(hier, residue_index=rnum,
                                     resolution=0).get_selected_particles()

            self.assertEquals(len(res), anums)
            self.assertEquals(
                IMP.atom.Residue(
                    IMP.atom.Atom(res[0]).get_parent()).get_residue_type(),
                IMP.pmi.tools.get_residue_type_from_one_letter_code(rname))
            res1 = IMP.atom.Selection(hier, residue_index=rnum,
                                      resolution=1).get_selected_particles()
            self.assertEquals(len(res1), 1)
            self.assertEquals(IMP.atom.Residue(res1[0]).get_residue_type(),
                              IMP.pmi.tools.get_residue_type_from_one_letter_code(rname))

        # check if res10 created correctly
        sel = IMP.atom.Selection(hier, residue_indexes=[1, 2], resolution=10)
        self.assertEquals(len(sel.get_selected_particles()), 1)

        sel = IMP.atom.Selection(
            hier, residue_indexes=range(5, 10), resolution=10)
        self.assertEquals(len(sel.get_selected_particles()), 1)

        sel = IMP.atom.Selection(hier, resolution=10)
        self.assertEquals(len(sel.get_selected_particles()), 4)

        sel = IMP.atom.Selection(hier, resolution=1)
        self.assertEquals(len(sel.get_selected_particles()), 9)

        sel1 = IMP.atom.Selection(hier, residue_index=1, resolution=10)
        sel2 = IMP.atom.Selection(hier, residue_index=2, resolution=10)

        self.assertEquals(sel1.get_selected_particles(),
                          sel2.get_selected_particles())
        sel1 = IMP.atom.Selection(hier, residue_index=1, resolution=10)
        sel2 = IMP.atom.Selection(hier, residue_index=5, resolution=10)
        self.assertNotEquals(sel1.get_selected_particles(),
                             sel2.get_selected_particles())

    def test_build_no0(self):
        """test building without resolution 0"""
        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})
        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)
        non_atomic_res = m1.get_residues() - atomic_res
        m1.add_representation(atomic_res, resolutions=[1, 10])
        m1.add_representation(non_atomic_res, resolutions=[10])
        hier = s.build()

        sel1 = IMP.atom.Selection(hier, resolution=1)
        self.assertEquals(len(sel1.get_selected_particles()), 9)

        sel10 = IMP.atom.Selection(hier, resolution=10)
        self.assertEquals(len(sel10.get_selected_particles()), 4)

    def test_build_nobeads(self):
        """test if add_representations populates the correct Residues"""
        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})
        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)
        m1.add_representation(atomic_res, resolutions=[0, 10])
        hier = s.build()
        sel0 = IMP.atom.Selection(hier, resolution=0)
        self.assertEquals(len(sel0.get_selected_particles()), 57)
        sel10 = IMP.atom.Selection(hier, resolution=10)
        self.assertEquals(len(sel10.get_selected_particles()), 2)

    def test_extra_breaks(self):
        """test adding extra breaks actually works"""
        s = IMP.pmi.topology.System()
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})
        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)
        m1.add_representation(m1, resolutions=10,bead_extra_breaks=['6'])
        hier = s.build()
        sel10 = IMP.atom.Selection(hier, resolution=10).get_selected_particles()
        self.assertEquals(len(sel10), 5)
        self.assertEquals(IMP.atom.Fragment(sel10[0]).get_residue_indexes(),
                          [1,2])
        self.assertEquals(IMP.atom.Fragment(sel10[1]).get_residue_indexes(),
                          [3,4])
        self.assertEquals(IMP.atom.Fragment(sel10[2]).get_residue_indexes(),
                          [5,6])
        self.assertEquals(IMP.atom.Fragment(sel10[3]).get_residue_indexes(),
                          [7,8,9])
        self.assertEquals(IMP.atom.Residue(sel10[4]).get_index(),
                          10)

    def test_create_copy(self):
        """Test creation of Copies"""
        s = IMP.pmi.topology.System()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})

        # create a molecule and add a copy
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"], chain_id='A')
        m2 = m1.create_copy(chain_id='B')
        self.assertEquals(st1.get_number_of_copies("Prot1"), 2)
        self.assertEquals(m2.get_name(), "Prot1")
        self.assertEquals([r.get_code() for r in m1.residues],
                          [r.get_code() for r in m2.residues])

    def test_create_clone(self):
        """Test creation and building of clone"""
        s = IMP.pmi.topology.System()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'),
            name_map={'Protein_1': 'Prot1',
                      'Protein_2': 'Prot2',
                      'Protein_3': 'Prot3'})

        # create a molecule and add a clone
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1", sequence=seqs["Prot1"], chain_id='A')
        m1c = m1.create_clone(chain_id='G')
        self.assertEquals(st1.get_number_of_copies("Prot1"), 2)
        self.assertEqual(st1.get_molecule("Prot1", 0), m1)
        self.assertEqual(st1.get_molecule("Prot1", 1), m1c)
        self.assertEqual(st1.get_molecule("Prot1", 'all'), [m1, m1c])

        # add structure+mixed representation to original
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)
        m1.add_representation(atomic_res, resolutions=[1, 10])
        m1.add_representation(m1.get_non_atomic_residues(), resolutions=[10])
        hier = s.build()

        # check that all resolutions created correctly for both copies
        sel1 = IMP.atom.Selection(
            hier, molecule='Prot1', resolution=1, copy_index=0).get_selected_particles()
        sel2 = IMP.atom.Selection(
            hier, molecule='Prot1', resolution=1, copy_index=1).get_selected_particles()
        sel3 = IMP.atom.Selection(
            hier, molecule='Prot1', resolution=1).get_selected_particles()
        self.assertEquals(len(sel1), len(sel2))
        self.assertEquals(set(sel1 + sel2), set(sel3))

        sel11 = IMP.atom.Selection(
            hier, molecule='Prot1', resolution=10, copy_index=0).get_selected_particles()
        sel21 = IMP.atom.Selection(
            hier, molecule='Prot1', resolution=10, copy_index=1).get_selected_particles()
        sel31 = IMP.atom.Selection(
            hier, molecule='Prot1', resolution=10).get_selected_particles()
        self.assertEquals(len(sel11), len(sel21))
        self.assertEquals(set(sel11 + sel21), set(sel31))

    def test_round_trip(self):
        def get_color(p):
            c = IMP.display.Colored(p).get_color()
            return [int(c.get_red() * 255), int(c.get_green() * 255), int(c.get_blue() * 255)]
        """Test RMF write/read representations"""
        base_res = 0
        bead_res = 1
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1", sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)
        non_atomic_res = m1.get_non_atomic_residues()
        m1.add_representation(
            atomic_res, resolutions=[base_res, bead_res], color='salmon')
        m1.add_representation(
            non_atomic_res, resolutions=[bead_res], color=(0.4,0.3,0.2))
        s.build()
        orig_hier = s.get_hierarchy()

        fname = self.get_tmp_file_name('test_round_trip.rmf3')
        rh = RMF.create_rmf_file(fname)
        IMP.rmf.add_hierarchy(rh, orig_hier)
        IMP.rmf.save_frame(rh)
        del rh

        rh2 = RMF.open_rmf_file_read_only(fname)
        h2 = IMP.rmf.create_hierarchies(rh2, mdl)[0]
        IMP.rmf.load_frame(rh2, 0)

        self.assertEqual(len(IMP.atom.get_leaves(orig_hier)),
                         len(IMP.atom.get_leaves(h2)))

        # check all coordinates and colors
        selA0 = IMP.atom.Selection(
            orig_hier, resolution=base_res).get_selected_particles()
        coordsA0 = [list(map(float, IMP.core.XYZ(p).get_coordinates()))
                    for p in selA0]
        colorsA0 = [get_color(p) for p in selA0]

        selB0 = IMP.atom.Selection(
            h2, resolution=base_res).get_selected_particles()
        coordsB0 = [list(map(float, IMP.core.XYZ(p).get_coordinates()))
                    for p in selB0]
        colorsB0 = [get_color(p) for p in selB0]

        self.assertEqual(coordsA0, coordsB0)
        self.assertEqual(colorsA0, colorsB0)

        selA1 = IMP.atom.Selection(
            orig_hier, resolution=bead_res).get_selected_particles()
        coordsA1 = [list(map(float, IMP.core.XYZ(p).get_coordinates()))
                    for p in selA1]
        selB1 = IMP.atom.Selection(
            h2, resolution=bead_res).get_selected_particles()
        coordsB1 = [list(map(float, IMP.core.XYZ(p).get_coordinates()))
                    for p in selB1]
        self.assertEqual(coordsA1, coordsB1)

    def test_setup_densities(self):
        """Test creating collective densities"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1", sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)
        non_atomic_res = m1.get_non_atomic_residues()

        fname = self.get_tmp_file_name('test_gmm')
        dres = 2
        m1.add_representation(atomic_res, resolutions=[0, 1],
                              density_residues_per_component=dres,
                              density_voxel_size=3.0,
                              density_prefix=fname)
        m1.add_representation(non_atomic_res, resolutions=[1])
        hier = s.build()

        selD = IMP.atom.Selection(hier, representation_type=IMP.atom.DENSITIES)
        self.assertEqual(len(selD.get_selected_particles()),
                         len(atomic_res) // dres + 1)

    def test_setup_beads_as_densities(self):
        """Test setup of individual density particles.
        This is mainly for flexible beads or all-atom simulations
        """
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1", sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54)

        fname = self.get_tmp_file_name('test_gmm')
        m1.add_representation(m1,
                              resolutions=[1],
                              setup_particles_as_densities=True)
        hier = s.build()
        selD = IMP.atom.Selection(hier, representation_type=IMP.atom.DENSITIES)
        self.assertEqual(selD.get_selected_particles(),
                         IMP.core.get_leaves(hier))

    def test_no_sequence(self):
        """Test automatic poly-A sequence when you add structure"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1")
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55, 63), offset=-54,
                                      soft_check=True)
        m1.add_representation(m1,
                              resolutions=[1])
        hier = s.build()
        expect_sequence = [IMP.pmi.tools.get_residue_type_from_one_letter_code(
            rname) for rname in 'QEAAVVKDL']
        ps = IMP.atom.Selection(hier).get_selected_particles()
        built_sequence = [IMP.atom.Residue(p).get_residue_type() for p in ps]
        self.assertEqual(expect_sequence, built_sequence)

    def test_ideal_helix(self):
        """Test you can build an ideal helix"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1", sequence='A' *25)
        m1.add_representation(m1[0:20],
                              resolutions=[1, 10],
                              ideal_helix=True,
                              density_prefix='hgmm',
                              density_voxel_size=0.0,
                              density_residues_per_component=10)
        m1.add_representation(m1[20:25], resolutions=[1])
        hier = s.build()

        # no idea how to test this
        selB = IMP.atom.Selection(hier, resolution=IMP.atom.ALL_RESOLUTIONS)
        selD = IMP.atom.Selection(
            hier, resolution=IMP.atom.ALL_RESOLUTIONS, representation_type=IMP.atom.DENSITIES)
        self.assertEqual(len(selB.get_selected_particles()), 20 +2+5)
        self.assertEqual(len(selD.get_selected_particles()), 3)
        os.unlink('hgmm.txt')

        self.assertEqual(m1.get_ideal_helices(), [m1[0:20]])

    def test_write_multistate(self):
        """Test you write multistate system with correct hierarchy"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))

        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(atomic_res,resolutions=0)
        st2 = s.create_state()
        m2 = st2.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m2.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54)
        m2.add_representation(atomic_res,resolutions=0)
        root_hier = s.build()

        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=root_hier,
                                              number_of_frames=0,
                                              number_of_best_scoring_models=0,
                                              global_output_directory='multistate_test/')
        rex.execute_macro()

        rh2 = RMF.open_rmf_file_read_only('multistate_test/initial.0.rmf3')
        hs = IMP.rmf.create_hierarchies(rh2,mdl)
        self.assertEqual(len(hs),1)
        states = IMP.atom.get_by_type(hs[0],IMP.atom.STATE_TYPE)
        self.assertEqual(len(states),2)

if __name__ == '__main__':
    IMP.test.main()
