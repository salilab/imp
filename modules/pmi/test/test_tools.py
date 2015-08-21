import IMP
import IMP.atom
import IMP.pmi
import IMP.pmi.topology as topology
import IMP.pmi.tools
import IMP.test


class ToolsTest(IMP.test.TestCase):
    def test_select_from_spec(self):
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(atomic_res,resolutions=[0,10])
        hier = s.build()
        test_hiers = IMP.pmi.tools.get_hierarchies_from_spec(m1)
        self.assertEqual(len(test_hiers),59)
if __name__ == '__main__':
    IMP.test.main()
