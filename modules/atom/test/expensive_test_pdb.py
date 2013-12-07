import StringIO
import IMP
import IMP.test
import IMP.atom


class PDBReadWriteTest(IMP.test.TestCase):

    def _test_round_trip(self, name, selector):
        m = IMP.kernel.Model()
        p = IMP.atom.read_pdb(self.get_input_file_name(name),
                              m, selector)
        n1 = len(IMP.atom.get_by_type(p, IMP.atom.ATOM_TYPE))
        sout = StringIO.StringIO()
        IMP.atom.write_pdb(p, sout)
        sin = StringIO.StringIO(sout.getvalue())
        p2 = IMP.atom.read_pdb(sin, m, selector)
        n2 = len(IMP.atom.get_by_type(p2, IMP.atom.ATOM_TYPE))
        self.assertEqual(n1, n2)
        self.assertGreater(n1, 0)

    def test_round_trips(self):
        """Testing that we can read and write various pdbs"""
        self._test_round_trip(
            "1d3d-protein.pdb",
            IMP.atom.NonWaterPDBSelector())
        self._test_round_trip(
            "1d3d-protein.pdb",
            IMP.atom.NonAlternativePDBSelector())
        self._test_round_trip("1DQK.pdb", IMP.atom.NonWaterPDBSelector())
        self._test_round_trip("1z5s_A.pdb", IMP.atom.NonWaterPDBSelector())
        self._test_round_trip("input.pdb", IMP.atom.NonWaterPDBSelector())
        self._test_round_trip(
            "protein_water.pdb",
            IMP.atom.NonWaterPDBSelector())
        self._test_round_trip(
            "protein_water.pdb",
            IMP.atom.NonAlternativePDBSelector())
        self._test_round_trip(
            "regression_0.pdb",
            IMP.atom.NonAlternativePDBSelector())
        self._test_round_trip(
            "single_dna.pdb",
            IMP.atom.NonAlternativePDBSelector())

    def test_read_het(self):
        """Check reading a pdb with one protein and a hetatm"""
        m = IMP.kernel.Model()

        #! read PDB
        mp = IMP.atom.read_pdb(self.open_input_file("1DQK.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 4060)
        # IMP.atom.show_molecular_hierarchy(mp)
        IMP.atom.show(mp)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        #self.assertEqual(bds.size(), 1020)
        IMP.atom.add_radii(mp)
        IMP.atom.show_molecular_hierarchy(mp)
        # read another PDB
        mp = IMP.atom.read_pdb(self.open_input_file("1aon.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 58870)


if __name__ == '__main__':
    IMP.test.main()
