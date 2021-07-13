import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_read(self):
        """Check get_next_residue() method"""
        m = IMP.Model()

        #! read PDB
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NonWaterPDBSelector())
        res = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        nres = IMP.atom.get_next_residue(IMP.atom.Residue(res[0]))

if __name__ == '__main__':
    IMP.test.main()
