import IMP
import IMP.test
import IMP.pmi
import IMP.pmi.dof
import IMP.pmi.topology
import IMP.pmi.macros
import IMP.mpi


class TestDOF(IMP.test.TestCase):
    #def setUp(self):
    #    self.rem = IMP.mpi.ReplicaExchange()

    def init_topology1(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))

        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(1,10),offset=-54)
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
                              chain_id='A',res_range=(1,10),offset=-54)
        a2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(1,13),offset=-179)
        a3 = m3.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='G',res_range=(1,10),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[1])
        m2.add_representation(a2,resolutions=[0,1])
        m2.add_representation(m2.get_non_atomic_residues(),resolutions=[1])
        m3.add_representation(a3,resolutions=[0,1])
        m3.add_representation(m3.get_non_atomic_residues(),resolutions=[1])
        s.build()
        return m1,m2,m3

    def test_mc_rigid_body(self):
        """Test creation of rigid body and nonrigid members"""
        mdl = IMP.Model()
        molecule = self.init_topology1(mdl)

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb_movers = dof.create_rigid_body(molecule,
                                          nonrigid_parts = molecule.get_non_atomic_residues(),
                                          name="test RB")
        mvs = dof.get_movers()
        self.assertEqual(len(rb_movers),4)
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=molecule.hier,
                                              monte_carlo_sample_objects=mvs,
                                              number_of_frames=2,
                                              test_mode=True)
                                              #replica_exchange_object=self.rem)
        rex.execute_macro()


    def test_mc_super_rigid_body(self):
        mdl = IMP.Model()
        mols = self.init_topology3(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb1_mov = dof.create_rigid_body(mols[0],
                                        nonrigid_parts = mols[0].get_non_atomic_residues())
        rb2_mov = dof.create_rigid_body(mols[1],
                                        nonrigid_parts = mols[1].get_non_atomic_residues())
        rb3_mov = dof.create_rigid_body(mols[2],
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
                              chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[1])
        m3 = m1.create_clone(chain_id='C')

        m2 = st1.create_molecule("Prot2",sequence=seqs["Protein_2"])
        a2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(1,13),offset=-179)
        m2.add_representation(a2,resolutions=[0,1])
        m2.add_representation(m2.get_non_atomic_residues(),resolutions=[1])
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

        ### test transformation propagates
        m1_leaves = m1.get_particles_at_all_resolutions()
        m3_leaves = m3.get_particles_at_all_resolutions()

        rbs,beads = IMP.pmi.tools.get_rbs_and_beads(m1_leaves)
        test_trans = IMP.algebra.get_random_local_transformation(IMP.algebra.Vector3D(0,0,0))
        IMP.core.transform(rbs[0],test_trans)
        mdl.update()

        for p1,p3 in zip(m1_leaves,m3_leaves):
            c1 = sym_trans*IMP.core.XYZ(p1).get_coordinates()
            c3 = IMP.core.XYZ(p3).get_coordinates()
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
                                      chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(atomic_res,resolutions=[0])
        hier = m1.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        md_ps = dof.setup_md(m1)
        #rex = IMP.pmi.macros.ReplicaExchange0(mdl,
        #                                      root_hier=hier,
        #                                      molecular_dynamics_sample_objects=md_ps,
        #                                      number_of_frames=2,
        #                                      test_mode=True,
        #                                      replica_exchange_object=self.rem)
        #rex.execute_macro()



if __name__ == '__main__':
    IMP.test.main()
