import IMP
import IMP.pmi
import IMP.pmi.topology
import IMP.test
import IMP.pmi.macros
import IMP.pmi.dof

class TestPMI2SimpleModel(IMP.test.TestCase):
    def test_it(self):

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences('input/seqs.fasta',
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        m2 = st1.create_molecule("Prot2",sequence=seqs["Prot2"])
        m3 = st1.create_molecule("Prot3",sequence=seqs["Prot3"])
        a1 = m1.add_structure('input/prot.pdb',
                              chain_id='A',res_range=(1,10),offset=-54)

        a2 = m2.add_structure('input/prot.pdb',
                              chain_id='B',res_range=(1,13),offset=-179)

        a3 = m3.add_structure('input/prot.pdb',
                              chain_id='G',res_range=(1,10),offset=-54)

        m1.add_representation(a1,resolutions=[0])
        m1.add_representation(resolutions=[1])
        m2.add_representation(a2,resolutions=[0])
        m2.add_representation(resolutions=[1])
        m3.add_representation(a3,resolutions=[0])
        m3.add_representation(resolutions=[1])


        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)

        rb1 = dof.create_rigid_body(m1)
        rb2 = dof.create_rigid_body(m2)
        rb3 = dof.create_rigid_body(m3)
        rb1.create_non_rigid_members(m1.get_non_atomic_residues())
        rb2.create_non_rigid_members(m2.get_non_atomic_residues())
        rb3.create_non_rigid_members(m3.get_non_atomic_residues())

        srb = dof.create_super_rigid_body([m1,m2,m3])

        rex=IMP.pmi.macros.ReplicaExchange0(mdl,root_hier=st1.get_hierarchy(),monte_carlo_sample_objects=dof)
        rex.execute_macro()


if __name__ == '__main__':
    IMP.test.main()
