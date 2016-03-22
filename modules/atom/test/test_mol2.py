from __future__ import print_function
import IMP
import IMP.test
import IMP.atom
import IMP.core
import io


class Tests(IMP.test.TestCase):

    def test_no_moltype(self):
        """Check reading a mol2 file with missing molecule type"""
        m = IMP.Model()

        root_d = IMP.atom.read_mol2(
            self.get_input_file_name("no-mol-type.mol2"),
            m, IMP.atom.AllMol2Selector())
        rs = IMP.atom.get_by_type(root_d, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(len(rs), 1)
        self.assertEqual(IMP.atom.Residue(rs[0]).get_residue_type(),
                         IMP.atom.UNK)

    def test_read(self):
        """Check reading a mol2 file containing small molecules"""
        m = IMP.Model()

        #! read Mol2
        root_d = IMP.atom.read_mol2(
            self.get_input_file_name("1d3d-ligands.mol2"),
            m, IMP.atom.AllMol2Selector())

        print(len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE)))
        self.assertEqual(
            146, len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE)))
        m2 = io.BytesIO()
        IMP.atom.write_mol2(root_d, m2)

        pdb = io.BytesIO()
        IMP.atom.write_pdb(root_d, pdb)
        # print m2.getvalue()
        # print pdb
        return

    def test_write(self):
        """Check that a mol2-created hierarchy can be written to a PDB"""
        m = IMP.Model()

        #! read Mol2
        root_d = IMP.atom.read_mol2(
            self.get_input_file_name("1d3d-ligands.mol2"),
            m, IMP.atom.AllMol2Selector())
        print(len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE)))
        self.assertEqual(
            146, len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE)))

        pdb = io.BytesIO()
        IMP.atom.write_pdb(root_d, pdb)
        print(pdb.getvalue())
        return

        m2 = IMP.Model()
        root_d2 = IMP.atom.read_mol2(
            self.get_input_file_name("1d3d-ligands.mol2"),
            m2, IMP.atom.NonhydrogenMol2Selector())

        IMP.atom.write_file(root_d2, self.get_tmp_file_name("2.mol2"))
        IMP.atom.write_file(root_d2, self.get_tmp_file_name("2.pdb"))

    def test_hao(self):
        """Check that written mol2s look right"""
        m = IMP.Model()

        #! read Mol2
        root_d = IMP.atom.read_mol2(
            self.get_input_file_name("1d3d-ligands.mol2"),
            m, IMP.atom.AllMol2Selector())
        out = io.BytesIO()
        IMP.atom.write_mol2(root_d, out)
        testout = open(
            self.get_input_file_name(
                "1d3d-ligands.out1.mol2"),
            "rb").read(
        )
        # print out.getvalue()
        # print " vs "
        # print testout
        self.assertEqual(out.getvalue(), testout)

        m = IMP.Model()
        root_d = IMP.atom.read_mol2(
            self.get_input_file_name("1d3d-ligands.mol2"),
            m, IMP.atom.NonHydrogenMol2Selector())
        print("test 2")

        out = io.BytesIO()
        IMP.atom.write_mol2(root_d, out)
        testout = open(
            self.get_input_file_name(
                "1d3d-ligands.out2.mol2"),
            "rb").read(
        )
        # print out.getvalue()
        # print " vs "
        # print testout
        self.assertEqual(out.getvalue(), testout)

if __name__ == '__main__':
    IMP.test.main()
