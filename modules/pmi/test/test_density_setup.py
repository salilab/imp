import IMP
import IMP.pmi
import IMP.pmi.dof
import IMP.pmi.topology
import IMP.pmi.macros
import IMP.test

class TestDensitySetup(IMP.test.TestCase):
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
        m1.add_representation(m1[:]-atomic_res,resolutions=[1])
        hier = m1.build()
        return m1

    def test_create_flex_density(self):
        """Test setup of all atom densities"""
        mdl = IMP.Model()
        mol = self.init_topology1(mdl)
        hier = mol.get_hierarchy()
        ds = IMP.pmi.macros.DensitySetup()
        ds.create_density(hier,num_components=0)
        IMP.atom.show_molecular_hierarchy(hier)
        #for p in IMP.core.get_leaves(hier):
        #    self.assertTrue(IMP.core.Gaussian.get_is_setup(p))

    def test_create_rigid_density(self):
        """Test setup of a set of densities for a body"""

    def test_create_densities_from_dof(self):
        """Test setup densities from DegreesOfFreedom object"""

if __name__ == '__main__':
    IMP.test.main()
