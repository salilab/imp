import IMP
import IMP.multifit
import IMP.test
import sys


class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.mdl = IMP.Model()
        self.mhs = []
        self.mhs.append(IMP.atom.read_pdb(
            self.get_input_file_name("1z5s_A.pdb"),
            self.mdl))
        self.mhs.append(IMP.atom.read_pdb(
            self.get_input_file_name("1z5s_B.pdb"),
            self.mdl))

        IMP.set_log_level(IMP.SILENT)

    def test_density_coarsening(self):
        dmap = IMP.em.read_map(self.get_input_file_name("1z5s_20.mrc"))
        num_beads = 4
        cmh = IMP.multifit.create_coarse_molecule_from_density(
            dmap, 0.2, num_beads, self.mdl, 5)
        self.assertEqual(len(IMP.core.get_leaves(cmh)), num_beads)

if __name__ == '__main__':
    IMP.test.main()
