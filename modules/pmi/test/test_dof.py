import IMP
import IMP.test
import IMP.pmi
import IMP.pmi.dof
import IMP.pmi.topology
import IMP.pmi.macros
import IMP.pmi.restraints.em
import os

try:
    import IMP.mpi
    rem = IMP.mpi.ReplicaExchange()
except ImportError:
    rem = None


class TestDOF(IMP.test.TestCase):
    def init_topology1(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(atomic_res,resolutions=[0,1,10])
        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[1])
        s.build()
        return m1
    def init_topology3(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))

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

        m3.add_representation(a3,resolutions=[0,1])
        m3.add_representation(m3.get_non_atomic_residues(),resolutions=[1])
        s.build()
        return m1,m2,m3

    def init_topology_densities(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))

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

        m3.add_representation(a3,resolutions=[0,1])
        m3.add_representation(m3.get_non_atomic_residues(),resolutions=[1], setup_particles_as_densities=True)
        s.build()
        return m1,m2,m3

    def test_mc_rigid_body(self):
        """Test creation of rigid body and nonrigid members"""
        mdl = IMP.Model()
        molecule = self.init_topology1(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb_movers,rb = dof.create_rigid_body(molecule,
                                             nonrigid_parts = molecule.get_non_atomic_residues(),
                                             name="test RB")
        mvs = dof.get_movers()
        self.assertEqual(len(rb_movers),4)
        all_members = rb.get_member_indexes()
        rigid_members = rb.get_rigid_members()
        num_nonrigid = len(all_members)-len(rigid_members)

        self.assertEqual(num_nonrigid,3)
        #                                   r0  r1  r10
        self.assertEqual(len(rigid_members),57 + 7 + 2)
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=molecule.get_hierarchy(),
                                              monte_carlo_sample_objects = dof.get_movers(),
                                              number_of_frames=2,
                                              test_mode=True,
                                              replica_exchange_object=rem)
        rex.execute_macro()
    def test_big_rigid_body(self):
        """test you can create a rigid body from 3 molecules"""
        mdl = IMP.Model()
        mols = self.init_topology3(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        mvs,rb = dof.create_rigid_body(mols,
                                    nonrigid_parts=[m.get_non_atomic_residues() for m in mols])
        self.assertEqual(len(mvs),1+3+3)
        all_members = rb.get_member_indexes()
        rigid_members = rb.get_rigid_members()
        num_nonrigid = len(all_members)-len(rigid_members)
        self.assertEqual(num_nonrigid,6)
        #                                      res0     res1
        self.assertEqual(len(rigid_members),57+110+57 + 7+13+7)


    def test_slice_rigid_body(self):
        """test you can create a rigid body from slices of molecules"""
        mdl = IMP.Model()
        mols = self.init_topology3(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        mvs,rb = dof.create_rigid_body([mols[0][:4],mols[1][:]],
                                    nonrigid_parts=mols[0][2:4])
        self.assertEqual(len(mvs),3)
        all_members = rb.get_member_indexes()
        rigid_members = rb.get_rigid_members()
        num_nonrigid = len(all_members)-len(rigid_members)
        self.assertEqual(num_nonrigid,2)
        #                                    res0     res1
        self.assertEqual(len(rigid_members),18+110 + 2+13)

    def test_rigid_body_with_densities(self):
        """Test still works when you add densities"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(atomic_res,
                              resolutions=[0,1,10],
                              density_prefix='tmpgmm',
                              density_residues_per_component=5)
        m1.add_representation(m1.get_non_atomic_residues(),
                              resolutions=[1],
                              setup_particles_as_densities=True)
        s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        mvs,rb = dof.create_rigid_body(m1,
                                    nonrigid_parts = m1.get_non_atomic_residues())
        self.assertEqual(len(mvs),4)
        all_members = rb.get_member_indexes()
        rigid_members = rb.get_rigid_members()
        num_nonrigid = len(all_members)-len(rigid_members)
        sel = IMP.atom.Selection(m1.get_hierarchy(),representation_type=IMP.atom.DENSITIES)
        self.assertEqual(num_nonrigid,3)
        #                                   r0  r1  r10   den
        self.assertEqual(len(rigid_members),57 + 7 + 2  + 2)
        os.unlink('tmpgmm.mrc')
        os.unlink('tmpgmm.txt')

    def test_mc_super_rigid_body(self):
        mdl = IMP.Model()
        mols = self.init_topology3(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb1_mov,rb1 = dof.create_rigid_body(mols[0],
                                        nonrigid_parts = mols[0].get_non_atomic_residues())
        rb2_mov,rb2 = dof.create_rigid_body(mols[1],
                                        nonrigid_parts = mols[1].get_non_atomic_residues())
        rb3_mov,rb3 = dof.create_rigid_body(mols[2],
                                        nonrigid_parts = mols[2].get_non_atomic_residues())
        srb_mover = dof.create_super_rigid_body(mols,chain_min_length=2,
                                                chain_max_length=2)
        ### rbX = dof.create_rigid_body([mols[0],mols[1]]) should fail
        # rb1:4, rb2:1, rb3:4, srbs:2
        mvs = dof.get_movers()
        self.assertEqual(len(mvs),11)


    def test_mc_flexible_beads(self):
        """Test setup of flexible beads"""
        mdl = IMP.Model()
        mol = self.init_topology1(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        fb_movers = dof.create_flexible_beads(mol.get_non_atomic_residues(),max_trans=1.0)
        self.assertEqual(len(fb_movers),3)
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=mol.get_hierarchy(),
                                              monte_carlo_sample_objects = dof.get_movers(),
                                              number_of_frames=2,
                                              test_mode=True,
                                              replica_exchange_object=rem)
        rex.execute_macro()

    def test_mc_flexible_beads3(self):
        """Test flex beads don't work if nothing passed"""
        mdl = IMP.Model()
        mols = self.init_topology3(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        fb_movers = dof.create_flexible_beads(mols[1].get_non_atomic_residues(),max_trans=1.0)
        self.assertEqual(len(fb_movers),0)

    def test_constraint_symmetry(self):
        """Test setup and activity of symmetry constraint"""
        ### create representation
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[1])
        m3 = m1.create_clone(chain_id='C')

        m2 = st1.create_molecule("Prot2",sequence=seqs["Protein_2"])
        a2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(180,192),offset=-179)
        m2.add_representation(a2,resolutions=[0,1])
        m4 = m2.create_clone(chain_id='D')
        root = s.build()

        ### create movers and constraints
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb1_movers = dof.create_rigid_body(m1,
                                           nonrigid_parts = m1.get_non_atomic_residues())
        rb2_movers = dof.create_rigid_body(m2,
                                           nonrigid_parts = m2.get_non_atomic_residues())
        dof.create_rigid_body(m3,
                              nonrigid_parts = m3.get_non_atomic_residues())
        dof.create_rigid_body(m4,
                              nonrigid_parts = m4.get_non_atomic_residues())

        sym_trans = IMP.algebra.get_random_local_transformation(IMP.algebra.Vector3D(0,0,0))
        dof.constrain_symmetry([m1,m2],[m3,m4],sym_trans)

        m1_leaves = IMP.pmi.tools.select_at_all_resolutions(m1.get_hierarchy())
        m3_leaves = IMP.pmi.tools.select_at_all_resolutions(m3.get_hierarchy())

        ### test symmetry initially correct
        mdl.update()
        for p1,p3 in zip(m1_leaves,m3_leaves):
            c1 = IMP.core.XYZ(p1).get_coordinates()
            c3 = sym_trans*IMP.core.XYZ(p3).get_coordinates()
            for i in range(3):
                self.assertAlmostEqual(c1[i],c3[i])

        ### test transformation propagates
        rbs,beads = IMP.pmi.tools.get_rbs_and_beads(m1_leaves)
        test_trans = IMP.algebra.get_random_local_transformation(IMP.algebra.Vector3D(0,0,0))
        IMP.core.transform(rbs[0],test_trans)
        mdl.update()

        for p1,p3 in zip(m1_leaves,m3_leaves):
            c1 = IMP.core.XYZ(p1).get_coordinates()
            c3 = sym_trans*IMP.core.XYZ(p3).get_coordinates()
            for i in range(3):
                self.assertAlmostEqual(c1[i],c3[i])

        #srb = dof.create_super_rigid_body([m1,m2])   # should be OK
        #srb = dof.create_super_rigid_body([m3,m4])   # should raise exception

    def test_mc_with_densities(self):
        pass

    def test_mc_kinematic(self):
        pass

    def test_md(self):
        """Test you can setup MD"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54)
        m1.add_representation(atomic_res,resolutions=[0])
        hier = m1.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        md_ps = dof.setup_md(m1)
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=hier,
                                              molecular_dynamics_sample_objects=md_ps,
                                              number_of_frames=2,
                                              test_mode=True,
                                              replica_exchange_object=rem)
        rex.execute_macro()

    def test_gaussian_rb(self):

        mdl = IMP.Model()
        m1, m2, m3 = self.init_topology_densities(mdl)
        densities = [r.get_hierarchy() for r in m3.get_non_atomic_residues()]
        gem_xtal = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                                                 self.get_input_file_name('prot_gmm.txt'),
                                                 target_is_rigid_body=True)

        em_rb = gem_xtal.get_rigid_body()
        em_rb.set_coordinates_are_optimized(False)

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(em_rb)

        self.assertTrue(em_rb in dof.get_rigid_bodies())
        self.assertEqual(len(dof.get_rigid_bodies()), 1)
        self.assertTrue(em_rb.get_coordinates_are_optimized())
        self.assertEqual(len(dof.get_movers()), 1)

    def test_rex_multistate(self):
        """Test you can do multi-state replica exchange"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1")
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54,
                                      soft_check=True)
        m1.add_representation(m1,resolutions=[1])
        st2 = s.create_state()
        m2 = st2.create_molecule("Prot1")
        atomic_res2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(55,63),offset=-54,
                                      soft_check=True)
        m2.add_representation(m2,resolutions=[1])
        hier = s.build()
        self.assertEqual(len(IMP.atom.get_by_type(hier,IMP.atom.STATE_TYPE)),2)

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(m1,nonrigid_parts = atomic_res)
        dof.create_rigid_body(m2,nonrigid_parts = atomic_res2)

        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=hier,
                                              monte_carlo_sample_objects=dof.get_movers(),
                                              number_of_frames=2,
                                              test_mode=True,
                                              replica_exchange_object=rem)
        rex.execute_macro()

if __name__ == '__main__':
    IMP.test.main()
