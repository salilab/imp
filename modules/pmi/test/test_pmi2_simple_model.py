import IMP
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.test
import IMP.pmi.macros
import IMP.pmi.dof

class TestPMI2SimpleModel(IMP.test.TestCase):

    def get_all_resolutions(self,h):
        par = h.get_parent()
        if IMP.atom.Representation.get_is_setup(par):
            rep = IMP.atom.Representation(par)
        elif IMP.atom.Representation.get_is_setup(par.get_parent()):
            rep = IMP.atom.Representation(par.get_parent())
        elif IMP.atom.Representation.get_is_setup(par.get_parent().get_parent()):
            rep = IMP.atom.Representation(par.get_parent().get_parent())
        else:
            print('WTF')
            exit()
        return rep.get_all_resolutions()

    def select_at_all_resolutions(self,h):
        all_res = self.get_all_resolutions(h)

    @IMP.test.expectedFailure
    def test_simple_model(self):
        """Test simple model with PMI2"""

        # setup system topology
        mdl = IMP.Model()
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

        m1.add_representation(a1,resolutions=[0])
        m1.add_representation(resolutions=[1])
        m2.add_representation(a2,resolutions=[0])
        m2.add_representation(resolutions=[1])
        m3.add_representation(a3,resolutions=[0])
        m3.add_representation(resolutions=[1])
        s.build()

        # setup degrees of freedom
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb1 = dof.create_rigid_body(m1)
        rb2 = dof.create_rigid_body(m2)
        rb3 = dof.create_rigid_body(m3)
        rb1.create_non_rigid_members(m1.get_non_atomic_residues())
        rb3.create_non_rigid_members(m3.get_non_atomic_residues())

        srb1 = dof.create_super_rigid_body([m1,m2,m3])
        srb2 = dof.create_super_rigid_body([m1,m2,m3],chain_min_length=2,chain_max_length=2)

        rex=IMP.pmi.macros.ReplicaExchange0(mdl,
                                            root_hier=st1.get_hierarchy(),
                                            degrees_of_freedom=dof,
                                            global_output_directory='output/',
                                            number_of_frames=10,
                                            monte_carlo_steps=1,
                                            write_initial_rmf=True,
                                            atomistic=True,
                                            number_of_best_scoring_models=1)
        rex.execute_macro()


if __name__ == '__main__':
    IMP.test.main()
