from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.saxs
import IMP.pmi
import IMP.pmi.restraints
import IMP.pmi.restraints.saxs

class Tests(IMP.test.TestCase):

    def get_exp_profile(self):
        return self.get_input_file_name('multi_A.pdb.dat')

    def setup_system(self):

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('multi_seq.fasta'),
                         name_map={'Protein_1':'Prot1'})

        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')

        m1_res = m1.add_structure(self.get_input_file_name('multi.pdb'),
                                  chain_id='A',offset=-54,
                                  model_num=0)

        m1.add_representation(m1_res,resolutions=[0])

        st1.build()
        return mdl, m1


    def test_SAXSRestraint(self):
        mdl, m1 = self.setup_system()
        saxs_rest = IMP.pmi.restraints.saxs.SAXSRestraint(m1, self.get_exp_profile(), ff_type=IMP.saxs.HEAVY_ATOMS)
        mdl.update()

        self.assertEqual(len(saxs_rest.particles), 256)
        self.assertAlmostEqual(saxs_rest.evaluate(), 0.016358, delta = 0.01)

        saxs_rest.add_to_model()

    def test_SAXSRestraint_residue(self):
        mdl, m1 = self.setup_system()
        saxs_rest2 = IMP.pmi.restraints.saxs.SAXSRestraint(m1, self.get_exp_profile(), ff_type=IMP.saxs.CA_ATOMS)
        mdl.update()

        self.assertEqual(len(saxs_rest2.particles), 31)
        self.assertAlmostEqual(saxs_rest2.evaluate(), 0.241081, delta = 0.01)


if __name__ == '__main__':
    IMP.test.main()
