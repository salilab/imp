import IMP
import IMP.pmi.topology
import IMP.pmi.restraints.basic
import IMP.test

class BasicTest(IMP.test.TestCase):
    def test_distance(self):
        """Test setup of distance restraint"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1 = st1.create_molecule("Prot1",chain_id='A',sequence=seqs["Prot1"])
        m2 = st1.create_molecule("Prot2",chain_id='B',sequence=seqs["Prot2"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,64),offset=-54)
        a2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(180,192),offset=-179)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1[:]-a1,resolutions=[1])
        m2.add_representation(a2,resolutions=[0,1])
        m3 = m2.create_clone('C')
        hier = s.build()


        dr = IMP.pmi.restraints.basic.DistanceRestraint(root_hier = hier,
                                                        tuple_selection1=(1,1,"Prot1",0),
                                                        tuple_selection2=(1,1,"Prot2",1),
                                                        distancemin=5,
                                                        distancemax=15)
        dr.add_to_model()
        rs = dr.get_restraint().get_restraints()
        self.assertEqual(len(rs),2)
        p1 = IMP.atom.Selection(hier,molecule="Prot1",resolution=1,
                                residue_index=1).get_selected_particles()[0]
        p2 = IMP.atom.Selection(hier,molecule="Prot2",resolution=1,
                                residue_index=1,copy_index=1).get_selected_particles()[0]
        self.assertEqual(set(rs[0].get_inputs()),set([p1,p2]))

if __name__ == '__main__':
    IMP.test.main()
