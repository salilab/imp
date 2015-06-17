import IMP
import IMP.atom
import IMP.pmi
import IMP.test
import IMP.pmi.topology as topology
import IMP.pmi.restraints.crosslinking_atomic
import IMP.pmi.io

class AtomicXLTest(IMP.test.TestCase):
    def test_atomic_xl(self):
        """ test PMI setup of atomic XL restraint """
        # create two states, each with two copies of the protein
        s=topology.System()
        seqs=topology.Sequences(self.get_input_file_name('multi_seq.fasta'),
                         name_map={'Protein_1':'Prot1'})
        # build state 1
        st1=s.create_state()
        m1=st1.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')
        atomic_res = m1.add_structure(self.get_input_file_name('multi.pdb'),
                                      chain_id='A',offset=-54,
                                      model_num=0)
        m1a = m1.create_copy(chain_id='G')
        m1a_res = m1a.add_structure(self.get_input_file_name('multi.pdb'),chain_id='G',offset=-54)
        m1.add_representation(atomic_res,resolutions=[0])
        m1a.add_representation(m1a_res,resolutions=[0])

        # build state 2
        st2=s.create_state()
        m2=st2.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')
        atomic_res=m2.add_structure(self.get_input_file_name('multi.pdb'),
                                    chain_id='A',offset=-54,
                                    model_num=1)
        m2a = m2.create_copy(chain_id='G')
        m2a_res = m2a.add_structure(self.get_input_file_name('multi.pdb'),chain_id='G',offset=-54)
        m2.add_representation(atomic_res,resolutions=[0])
        m2.add_representation(m2a_res,resolutions=[0])
        hier = s.build(merge_type="backbone")

        # pass hierarchy and fake data to the restraint
        xls = IMP.pmi.io.CrossLinkData()
        xls.add_cross_link(0,
                           {'molecule':'Prot1','residue_index':7},
                           {'molecule':'Prot1','residue_index':39},
                           1)
        xl = IMP.pmi.restraints.crosslinking_atomic.AtomicCrossLinkMSRestraint(hier,xls,nstates=2)

        # check that you created 8 restraints (2 copies => 4 restraints. x2 states)
        rs=xl.get_restraint_set()
        self.assertEqual(rs.get_number_of_restraints(),1)
        xlrs=IMP.isd.AtomicCrossLinkMSRestraint.get_from(rs.get_restraint(0))
        self.assertIsInstance(xlrs,IMP.isd.AtomicCrossLinkMSRestraint)
        self.assertEqual(xlrs.get_number_of_contributions(),8)


if __name__ == '__main__':
    IMP.test.main()
