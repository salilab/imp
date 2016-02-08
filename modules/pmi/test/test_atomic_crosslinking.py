import IMP
import IMP.atom
import IMP.pmi
import IMP.test
import IMP.pmi.topology
import IMP.pmi.restraints.crosslinking
import IMP.pmi.io.crosslink
import sys,os

class AtomicXLTest(IMP.test.TestCase):
    def test_atomic_xl(self):
        """ test PMI setup of atomic XL restraint """

        # fake data
        tname = self.get_tmp_file_name("test.txt")
        with open(tname, "w") as fh:
            fh.write("prot1,res1,prot2,res2\nProt1,7,Prot1,39\n")

        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("prot1")
        cldbkc.set_protein2_key("prot2")
        cldbkc.set_residue1_key("res1")
        cldbkc.set_residue2_key("res2")
        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(tname)

        self.assertEqual(cldb.get_number_of_xlid(),1)

        # create two states, each with two copies of the protein
        s = IMP.pmi.topology.System()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('multi_seq.fasta'),
                         name_map={'Protein_1':'Prot1'})
        # build state 1
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')
        m1_res = m1.add_structure(self.get_input_file_name('multi.pdb'),
                                  chain_id='A',offset=-54,
                                  model_num=0)
        m1.add_representation(m1_res,resolutions=[0])

        m1a = m1.create_copy(chain_id='G')
        m1a_res = m1a.add_structure(self.get_input_file_name('multi.pdb'),chain_id='G',offset=-54)
        m1a.add_representation(m1a_res,resolutions=[0])

        # build state 2
        st2 = s.create_state()
        m2 = st2.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')
        m2_res = m2.add_structure(self.get_input_file_name('multi.pdb'),
                                  chain_id='A',offset=-54,
                                  model_num=1)
        m2.add_representation(m2_res,resolutions=[0])

        m2a = m2.create_copy(chain_id='G')
        m2a_res = m2a.add_structure(self.get_input_file_name('multi.pdb'),chain_id='G',offset=-54)
        m2a.add_representation(m2a_res,resolutions=[0])

        hier = s.build()

        #IMP.atom.show_molecular_hierarchy(hier)
        xl = IMP.pmi.restraints.crosslinking.AtomicCrossLinkMSRestraint(hier,
                                                                        cldb,
                                                                        nstates=[0,1],
                                                                        atom_type="CA")

        # check that you created 8 restraints:
        #  Each state: A1-B1, A1-B2, A2-B1, A2-B2
        rs=xl.get_restraint_set()
        self.assertEqual(rs.get_number_of_restraints(),1)
        xlrs=IMP.isd.AtomicCrossLinkMSRestraint.get_from(rs.get_restraint(0))
        self.assertIsInstance(xlrs,IMP.isd.AtomicCrossLinkMSRestraint)
        self.assertEqual(xlrs.get_number_of_contributions(),8)


if __name__ == '__main__':
    IMP.test.main()
