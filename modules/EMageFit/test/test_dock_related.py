import IMP
import IMP.test
import IMP.atom as atom
import sys
import os


class TestDockRelated(IMP.test.ApplicationTestCase):

    def test_filter_transformations(self):
        """
            Check if the filtered conformation are the conformations that I
            computed before
        """
        try:
            import subprocess
            import IMP.em2d.buildxlinks as bx
        except ImportError as e:
            self.skipTest(str(e))

        dock = self.import_python_application('emagefit_dock')
        sel = atom.NonWaterNonHydrogenPDBSelector()
        ligand = IMP.kernel.Model()
        fn_ligand = self.get_input_file_name("3sfdB-3sfdA_initial_docking.pdb")
        h_ligand = atom.read_pdb(fn_ligand, ligand, sel)
        rb_ligand = atom.create_rigid_body(h_ligand)
        receptor = IMP.kernel.Model()
        fn_receptor = self.get_input_file_name("3sfdB.pdb")
        h_receptor = atom.read_pdb(fn_receptor, receptor, sel)
        # read_hex_transformations
        fn = self.get_input_file_name("hex_solutions_3sfdB-3sfdA.txt")
        residue_receptor = 23
        residue_ligand = 456
        distance = 30
        xl = bx.Xlink("3sfdB", "B", residue_receptor,
                      "3sfdA", "A", residue_ligand, distance)

        xlinks_list = [xl]
        fn_filtered = "filtered_transforms.txt"
        dock.filter_docking_results(h_receptor, h_ligand, xlinks_list,
                                    fn, fn_filtered)
        fn_stored = self.get_input_file_name(
            "hex_solutions_3sfdB-3sfdA_filtered.txt")
        filtered = dock.read_hex_transforms(fn_filtered)
        stored = dock.read_hex_transforms(fn_stored)
        # check that the filtered transforms match the stored ones
        self.assertEqual(len(filtered), len(stored))
        for Tf, Ts in zip(filtered, stored):
            tf = Tf.get_translation()
            ts = Ts.get_translation()
            qf = Tf.get_rotation().get_quaternion()
            qs = Ts.get_rotation().get_quaternion()
            for k in range(3):
                self.assertAlmostEqual(tf[k], ts[k])
            for k in range(4):
                self.assertAlmostEqual(qf[k], qs[k])
        os.remove(fn_filtered)

    def test_import(self):
        """
            Check that the module is at least imported correctly
        """
        try:
            import subprocess
        except ImportError as e:
            self.skipTest(str(e))
        self.import_python_application('emagefit_dock')

if __name__ == '__main__':
    IMP.test.main()
