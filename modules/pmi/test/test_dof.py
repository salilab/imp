import IMP
import IMP.pmi
import IMP.pmi.dof
import IMP.pmi.topology
import IMP.test


class TestDOF(IMP.test.TestCase):
    def init_topology1(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A',res_range=(1,10),offset=-54)
        m1.add_representation(atomic_res,resolutions=[0])
        m1.add_representation(resolutions=[1])
        hier = m1.build()
        return m1
    def init_topology3(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1 = st1.create_molecule("Prot1",sequence=seqs["Prot1"])
        m2 = st1.create_molecule("Prot2",sequence=seqs["Prot2"])
        m3 = st1.create_molecule("Prot3",sequence=seqs["Prot3"])
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
        hier = s.build()
        return m1,m2,m3
    '''
    def test_constraint_symmetry(self):
        hierarchy=self.init_topology()
        dof=IMP.pmi.dof.DegreesOfFreedom()
        selections=[]
        s0=IMP.atom.Selection(hierarchy,molecule=?,copy=0)
        s1=IMP.atom.Selection(hierarchy,molecule=?,copy=1)
        dof.create_symmetry_contraint([s0,s1],IMP.core.Transformation)
    '''
    '''
    def test_mc_compound_body(self):
        # compund body is a mix of rigid and flexible parts
        # Do we use System???? Do we need a new decorator that says
        # where the structure is coming from????
        # IMP.atom.Source(p)
        # IMP.atom.Source.get_pdb_id()
        # IMP.atom.Source.get_pdb_id()
        # source can be Modeller, PDB, emdb, Coarse-grained, No-source
        # Invent StringKeys
        hierarchy=self.init_topology()
        dof=IMP.pmi.dof.DegreesOfFreedom()
        s=IMP.atom.Selection(hierarchy,molecule=?,resid=range(1,10))
        structured_handle,unstructures_handle=dof.create_compound_body(s)
    '''

    def test_mc_rigid_body(self):
        """Test creation of rigid body and nonrigid members"""
        mdl = IMP.Model()
        molecule = self.init_topology1(mdl)
        hier = molecule.get_hierarchy()
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)

        sel_nonrigid = IMP.atom.Selection(hier,residue_indexes=[3,4,10])
        rigid_body = dof.create_rigid_body(hier)
        rigid_body.create_non_rigid_members(sel_nonrigid)

        rb = rigid_body.get_rigid_body()
        mvs = rigid_body.get_movers()
        self.assertEqual(len(mvs),4)

    def test_mc_super_rigid_body(self):
        mdl = IMP.Model()
        m1,m2,m3 = self.init_topology3(mdl)
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)

        h1 = m1.get_hierarchy()
        h2 = m1.get_hierarchy()
        h3 = m1.get_hierarchy()
        srb = dof.create_super_rigid_body([h1,h2,h3],chain_min_length=2,chain_max_length=2)
        self.assertEqual(len(srb.get_movers()),2)
        print(srb.get_movers())

    def test_mc_flexible_beads(self):
        pass

    def test_mc_kinematic(self):
        pass

    def test_md(self):
        pass

if __name__ == '__main__':
    IMP.test.main()
