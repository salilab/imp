import IMP
import IMP.atom
import IMP.pmi
import IMP.pmi.topology as topology
import IMP.test
import RMF
import IMP.rmf

def get_atomic_residue_list(residues):
    r1=[]
    for r in residues:
        ps=r.get_hierarchy().get_children()
        if len(ps)==0:
            r1.append('-')
        else:
            r1.append(IMP.atom.get_one_letter_code(r.get_residue_type()))
    return ''.join(r1)

class RepresentationNewTest(IMP.test.TestCase):

    def test_read_sequences(self):
        '''Test if the sequence reader returns correct strings'''
        # test without name map
        seqs0=topology.Sequences(self.get_input_file_name('seqs.fasta'))
        self.assertEqual(len(seqs0),3)
        self.assertEqual(seqs0['Protein_1'],'QEALVVKDLL')
        self.assertEqual(seqs0['Protein_2'],'PEEDILKYVSYTL')
        self.assertEqual(seqs0['Protein_3'],'QEALVVKDLL')

        # test with name map
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})
        self.assertEqual(len(seqs),3)
        self.assertEqual(seqs['Prot1'],'QEALVVKDLL')
        self.assertEqual(seqs['Prot2'],'PEEDILKYVSYTL')
        self.assertEqual(seqs['Prot3'],'QEALVVKDLL')

    def test_system_base(self):
        '''Test systembase functions like create hierarchy and create child'''
        sb=topology.SystemBase()
        root=sb._create_hierarchy()
        child=sb._create_child(root)
        self.assertEqual(child.get_parent(),root)

    def test_create_states(self):
        '''Test State-creation from System'''
        s=topology.System()
        for i in range(10):
            self.assertEqual(s.get_number_of_states(),i)
            st=s.create_state()
            self.assertEqual(st.get_hierarchy().get_parent(),s.get_hierarchy())
            self.assertEqual(st.mdl,s.mdl)
        self.assertEqual(s.get_number_of_states(),10)


    def test_create_molecules(self):
        '''Test Molecule creation from State'''
        s=topology.System()
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        # create state 1 with 2 molecules
        st=s.create_state()
        m1=st.create_molecule("Prot1",sequence=seqs["Prot1"])
        m2=st.create_molecule("Prot2",sequence=seqs["Prot2"])
        self.assertEqual(m1.get_hierarchy().get_parent(),st.get_hierarchy())
        self.assertEqual(m2.get_hierarchy().get_parent(),st.get_hierarchy())
        self.assertEqual(m1.mdl,st.mdl)
        self.assertEqual(m2.mdl,st.mdl)
        self.assertEqual(m1.get_name(),"Prot1")
        self.assertEqual(m2.get_name(),"Prot2")
        self.assertEqual(len(st.get_hierarchy().get_children()),2)

        # create state 2 with one molecule
        st2=s.create_state()
        m3=st2.create_molecule("Prot3",sequence=seqs["Prot3"])
        self.assertEqual(m3.get_hierarchy().get_parent(),st2.get_hierarchy())
        self.assertEqual(m3.mdl,st2.mdl)
        self.assertEqual(m3.get_name(),"Prot3")
        self.assertEqual(len(st2.get_hierarchy().get_children()),1)

        # test if sequences are OK
        self.assertEqual(''.join(r.get_code() for r in m1.residues),seqs["Prot1"])
        self.assertEqual(''.join(r.get_code() for r in m2.residues),seqs["Prot2"])
        self.assertEqual(''.join(r.get_code() for r in m3.residues),seqs["Prot3"])

    def test_add_structure(self):
        '''Test adding partial structure data to a molecule'''

        s=topology.System()
        st1=s.create_state()
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1=st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        m2=st1.create_molecule("Prot2",sequence=seqs["Prot2"])
        res1=m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(1,10),offset=-54)
        res2=m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(1,13),offset=-179)

        # check that the molecule residues have the right info
        rlist1=get_atomic_residue_list(m1.residues)
        rlist2=get_atomic_residue_list(m2.residues)
        self.assertEqual(rlist1,'QE--VVKDL-')
        self.assertEqual(rlist2,'PEEDILKYVSYTL')

        # check that the returned Residue index sets are correct
        self.assertEqual(res1,set([m1.residues[i] for i in (0,1,4,5,6,7,8)]))
        self.assertEqual(res2,set([m2.residues[i] for i in range(0,13)]))

    def test_get_atomic_non_atomic_residues(self):
        '''test if, adding a structure, you get the atomic and non atomic residues sets
        correctly'''
        s=topology.System()
        st1=s.create_state()
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2'})

        m1=st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        m2=st1.create_molecule("Prot2",sequence=seqs["Prot2"])
        res1=m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(1,10),offset=-54)

        m1_atomic_residues=m1.get_atomic_residues()
        m1_non_atomic_residues=m1.get_non_atomic_residues()
        m1_all_residues=m1.get_residues()
        m2_atomic_residues=m2.get_atomic_residues()
        m2_non_atomic_residues=m2.get_non_atomic_residues()
        m2_all_residues=m2.get_residues()

        self.assertEqual(m1_atomic_residues,m1_all_residues-m1_non_atomic_residues)
        self.assertEqual(m2_atomic_residues,m2_all_residues-m2_non_atomic_residues)

    def test_residue_access(self):
        '''test functions to retrieve residues'''
        s=topology.System()
        st1=s.create_state()
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1=st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        self.assertEqual(m1[:],set(m1.residues[:]))
        self.assertEqual(m1[1],m1.residues[1])
        self.assertEqual(m1[1:5],set(m1.residues[1:5]))
        self.assertEqual(m1['1'],m1.residues[0])
        self.assertEqual(m1.residue_range(1,5),set(m1.residues[1:5]))
        self.assertEqual(m1.residue_range('2','6'),set(m1.residues[1:5]))
        inv=m1[:]-m1[1:5]
        self.assertEqual(inv,set([m1.residues[0]]+m1.residues[5:10]))

    def test_add_representation(self):
        '''test if add_representations propulates the correct Residues'''
        s=topology.System()
        st1=s.create_state()
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})
        m1=st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        atomic_res=m1.add_structure(self.get_input_file_name('prot.pdb'),
                                    chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(resolutions=[1])
        m1.add_representation(atomic_res,resolutions=[0])
        for na in (0,1,4,5,6,7,8):
            self.assertEqual(m1[na].representations['balls'],set([0,1]))
        for nna in (2,3,9):
            self.assertEqual(m1[nna].representations['balls'],set([1]))

    def test_build_system(self):
        s=topology.System()
        st1=s.create_state()
        seqs=topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})
        m1=st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        atomic_res=m1.add_structure(self.get_input_file_name('prot.pdb'),chain_id='A',
                                    res_range=(1,10),offset=-54)
        non_atomic_res=m1.get_residues()-atomic_res

        #m1.add_representation(m1[:]-atomic_res,resolutions=[1])
        m1.add_representation(atomic_res,resolutions=[0,1,10])
        m1.add_representation(non_atomic_res,resolutions=[10])
        hier = m1.build(merge_type="backbone")
        frags = hier.get_children()

        # check names
        self.assertEquals(hier.get_name(),'Prot1')
        self.assertEquals(m1.__repr__(),'System.State_0.Prot1.0')

        # check if res0,1 created correctly
        for rnum,rname,anums in zip((1,2,5,6,7,8,9),'QEVVKDL',(9,9,7,7,9,8,8)):
            res = IMP.atom.Selection(hier,residue_index=rnum,
                                     resolution=0).get_selected_particles()
            self.assertEquals(len(res),anums)
            self.assertEquals(IMP.atom.Residue(IMP.atom.Atom(res[0]).get_parent()).get_residue_type(),
                              topology.get_residue_type_from_one_letter_code(rname))
            res1 = IMP.atom.Selection(hier,residue_index=rnum,
                                      resolution=1).get_selected_particles()
            self.assertEquals(len(res1),1)
            self.assertEquals(IMP.atom.Residue(res1[0]).get_residue_type(),
                              topology.get_residue_type_from_one_letter_code(rname))

        # check if res10 created correctly
        sel = IMP.atom.Selection(hier,residue_indexes=[1,2],resolution=10)
        self.assertEquals(len(sel.get_selected_particles()),1)
        sel = IMP.atom.Selection(hier,residue_indexes=range(5,10),resolution=10)
        self.assertEquals(len(sel.get_selected_particles()),1)
        sel = IMP.atom.Selection(hier,resolution=10)
        self.assertEquals(len(sel.get_selected_particles()),4)
        sel = IMP.atom.Selection(hier,resolution=1)
        self.assertEquals(len(sel.get_selected_particles()),9)
        sel1 = IMP.atom.Selection(hier,residue_index=1,resolution=10)
        sel2 = IMP.atom.Selection(hier,residue_index=2,resolution=10)
        self.assertEquals(sel1.get_selected_particles(),sel2.get_selected_particles())
        sel1 = IMP.atom.Selection(hier,residue_index=1,resolution=10)
        sel2 = IMP.atom.Selection(hier,residue_index=5,resolution=10)
        self.assertNotEquals(sel1.get_selected_particles(),sel2.get_selected_particles())

    def test_create_copy(self):
        '''Test creation of Copies'''
        s = topology.System()
        seqs = topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        # create a molecule and add a copy
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')
        m2 = m1.create_copy(chain_id='B')
        self.assertEquals(st1.get_number_of_copies("Prot1"),2)
        self.assertEquals(m2.get_name(),"Prot1")
        self.assertEquals([r.get_code() for r in m1.residues],
                          [r.get_code() for r in m2.residues])

    def test_create_clone(self):
        '''Test creation and building of alone'''
        s = topology.System()
        seqs = topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        # create a molecule and add a clone
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"],chain_id='A')
        m1.create_clone(chain_id='G')
        self.assertEquals(st1.get_number_of_copies("Prot1"),2)

        # add structure+mixed representation to original
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(atomic_res,resolutions=[1,10])
        m1.add_representation(m1.get_non_atomic_residues(),resolutions=[10])
        hier = s.build(merge_type="backbone")

        # check that all resolutions created correctly for both copies
        sel1=IMP.atom.Selection(hier,molecule='Prot1',resolution=1,copy_index=0).get_selected_particles()
        sel2=IMP.atom.Selection(hier,molecule='Prot1',resolution=1,copy_index=1).get_selected_particles()
        sel3=IMP.atom.Selection(hier,molecule='Prot1',resolution=1).get_selected_particles()
        self.assertEquals(len(sel1),len(sel2))
        self.assertEquals(set(sel1+sel2),set(sel3))

        sel11=IMP.atom.Selection(hier,molecule='Prot1',resolution=10,copy_index=0).get_selected_particles()
        sel21=IMP.atom.Selection(hier,molecule='Prot1',resolution=10,copy_index=1).get_selected_particles()
        sel31=IMP.atom.Selection(hier,molecule='Prot1',resolution=10).get_selected_particles()
        self.assertEquals(len(sel11),len(sel21))
        self.assertEquals(set(sel11+sel21),set(sel31))

    def test_round_trip(self):
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'))
        m1 = st1.create_molecule("Prot1",sequence=seqs["Protein_1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(a1,resolutions=[0])
        m1.add_representation(resolutions=[1])
        s.build()
        orig_hier = s.get_hierarchy()

        fname = self.get_tmp_file_name('test_round_trip.rmf3')

        rh = RMF.create_rmf_file(fname)
        IMP.rmf.add_hierarchy(rh, orig_hier)
        IMP.rmf.save_frame(rh)
        del rh

        rh2 = RMF.open_rmf_file_read_only(fname)
        h2 = IMP.rmf.create_hierarchies(rh2,mdl)[0]
        IMP.rmf.load_frame(rh2,0)

        self.assertEqual(len(IMP.atom.get_leaves(orig_hier)),
                         len(IMP.atom.get_leaves(h2)))

        # check all coordinates
        selA0 = IMP.atom.Selection(orig_hier,resolution=0).get_selected_particles()
        coordsA0 = [map(float,IMP.core.XYZ(p).get_coordinates()) for p in selA0]
        selB0 = IMP.atom.Selection(h2,resolution=0).get_selected_particles()
        coordsB0 = [map(float,IMP.core.XYZ(p).get_coordinates()) for p in selB0]
        self.assertEqual(coordsA0,coordsB0)

        selA1 = IMP.atom.Selection(orig_hier,resolution=1).get_selected_particles()
        coordsA1 = [map(float,IMP.core.XYZ(p).get_coordinates()) for p in selA1]
        selB1 = IMP.atom.Selection(h2,resolution=1).get_selected_particles()
        coordsB1 = [map(float,IMP.core.XYZ(p).get_coordinates()) for p in selB1]
        self.assertEqual(coordsA1,coordsB1)

if __name__ == '__main__':
    IMP.test.main()
