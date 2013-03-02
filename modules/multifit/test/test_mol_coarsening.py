import IMP
import IMP.multifit
import IMP.test
import sys
class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.mdl=IMP.Model()
        self.mhs=[]
        self.mhs.append(IMP.atom.read_pdb(
            self.get_input_file_name("1z5s_A.pdb"),
            self.mdl))
        self.mhs.append(IMP.atom.read_pdb(
            self.get_input_file_name("1z5s_B.pdb"),
            self.mdl))

        IMP.base.set_log_level(IMP.base.SILENT)
    def test_coarsening(self):
        cmhs=IMP.multifit.create_coarse_molecules_from_molecules(
            self.mhs,30,self.mdl,5)
        print len(IMP.core.get_leaves(cmhs[0])),len(IMP.atom.get_by_type(self.mhs[0],IMP.atom.RESIDUE_TYPE))
        print len(IMP.core.get_leaves(cmhs[1])),len(IMP.atom.get_by_type(self.mhs[1],IMP.atom.RESIDUE_TYPE))
        self.assertEqual(len(IMP.core.get_leaves(cmhs[0])),5)
        self.assertEqual(len(IMP.core.get_leaves(cmhs[1])),2)
    def test_density_coarsening(self):
        dmap=IMP.em.read_map( self.get_input_file_name("1z5s_20.mrc"))
        num_beads=4
        cmh=IMP.multifit.create_coarse_molecule_from_density(
            dmap,0.2,num_beads,self.mdl,5)
        self.assertEqual(len(IMP.core.get_leaves(cmh)),num_beads)

if __name__ == '__main__':
    IMP.test.main()
