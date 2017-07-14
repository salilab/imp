import os
import IMP
import IMP.test
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.macros
import IMP.pmi.analysis



class Tests(IMP.test.TestCase):
    def setUp(self):
        mdl = IMP.Model()
        IMP.test.TestCase.setUp(self)
        reader = IMP.pmi.topology.TopologyReader(self.get_input_file_name('1mda/1mda_topology.dat'),
                                                 pdb_dir =   self.get_input_file_name('1mda/'),
                                                 fasta_dir = self.get_input_file_name('1mda/'),
                                                 gmm_dir =   self.get_input_file_name('1mda/gmm'))

        bs = IMP.pmi.macros.BuildSystem(mdl)
        bs.add_state(reader)
        self.hier, self.dof = bs.execute_macro(max_rb_trans=4.0, max_rb_rot=0.4, max_bead_trans=4.0, max_srb_trans=4.0,max_srb_rot=0.4)
        self.mols=IMP.pmi.tools.get_molecules(self.hier)

    def test_rmsd_simple(self):
        """Test get_particle_infos_for_pdb_writing with no particles"""
        rmsd=IMP.pmi.analysis.RMSD(self.hier,self.hier,[mol.get_name() for mol in self.mols],dynamic0=True,dynamic1=False)
        total_rmsd,best_assignments=rmsd.get_rmsd_and_assigments()
        self.assertEqual(total_rmsd,0.0)

        for ba in best_assignments:
            self.assertEqual(ba[0],ba[1])
        out=rmsd.get_output()

    def test_swap_copies(self):
        pass

if __name__ == '__main__':
    IMP.test.main()
