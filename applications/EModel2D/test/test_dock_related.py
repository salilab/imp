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

        dock = self.import_python_application('em2d_docking.py')
        sel = atom.NonWaterNonHydrogenPDBSelector()
        ligand = IMP.Model()
        fn_ligand = self.get_input_file_name("3sfdA.pdb")
        h_ligand = atom.read_pdb(fn_ligand, ligand, sel)
        rb_ligand = atom.create_rigid_body(h_ligand)
        receptor = IMP.Model()
        fn_receptor = self.get_input_file_name("3sfdB.pdb")
        h_receptor = atom.read_pdb(fn_receptor, receptor, sel)
        # read_hex_transformations
        fn = self.get_input_file_name("hex_solutions_3sfdB-3sfdA.txt")
        residue_receptor = 23
        residue_ligand = 456
        distance = 30
        pair_xlinks = [(residue_receptor, residue_ligand, distance)]
        fn_filtered = self.get_input_file_name("filtered_transforms.txt")
        dock.filter_docking_results(h_receptor, h_ligand, pair_xlinks,
                                            fn, fn_filtered)
        fn_stored = self.get_input_file_name(
                                    "hex_solutions_3sfdB-3sfdA_filtered.txt")
        filtered = dock.read_hex_transforms(fn_filtered)
        stored = dock.read_hex_transforms(fn_stored)
        # check that the filtered transforms match the stored ones
        self.assertEqual(len(filtered), len(stored))
        for Tf, Ts  in zip(filtered, stored):
            tf = Tf.get_translation()
            ts = Ts.get_translation()
            qf = Tf.get_rotation().get_quaternion()
            qs = Ts.get_rotation().get_quaternion()
            for k in range(3):
                self.assertAlmostEqual(tf[k], ts[k])
            for k in range(4):
                self.assertAlmostEqual(qf[k], qs[k])
        os.remove(fn_filtered)

if __name__ == '__main__':
    IMP.test.main()
